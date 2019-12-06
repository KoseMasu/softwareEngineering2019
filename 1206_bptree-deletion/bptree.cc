#include "bptree.h"
#include <vector>
#include <iostream>
#include <sys/time.h>
#define KEY_RANGE (1000*1000*100)
int Size_data = 1000*100;
vector<uint> Key_vector;

bool is_unique(uint key)
{
  for (auto itr = Key_vector.begin(); itr != Key_vector.end(); itr++) {
    if (*itr == key) return false;
  }
  return true;
}

void
gen_rand_list(uint max)
{
  for (uint i = 0; i < max; i++) {
    uint key = rand() % KEY_RANGE;
    if (is_unique(key) == false) {
      i--; continue;
    }
    Key_vector.push_back(key);
  }
}

bool is_internal(NODE *node)
{
  if (node->isLeaf == false) return true;
  else return false;
}

bool is_leaf(NODE *node)
{
  if (node->isLeaf == true) return true;
  else return false;
}

void
print_performance(struct timeval begin, struct timeval end)
{
  long diff = (end.tv_sec - begin.tv_sec) * 1000 * 1000 + (end.tv_usec - begin.tv_usec);
  printf("%10.0f req/sec (lat:%7ld usec)\n", ((double)Size_data) / ((double)diff/1000.0/1000.0), diff);
}

struct timeval
cur_time(void)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return t;
}

void
init_vector(void)
{
  /* sorted data
  for (int i = Size_data - 1; i >= 0; i--) {
    int key = i;
    Key_vector.push_back(key);
  }
  */

  unsigned int now = (unsigned int)time(0);
  srand(now);
  gen_rand_list(Size_data);
}

void
print_tree_core(NODE *n)
{
  printf("["); 
  //printf("<%d>", n->nkey);
  for (int i = 0; i < n->nkey; i++) {
    if (!n->isLeaf) print_tree_core(n->child[i]); 
    printf("%d", n->key[i]); 
    fflush(stdout);
    if (i != n->nkey-1 && n->isLeaf) putchar(' ');
  }
  if (!n->isLeaf) print_tree_core(n->child[n->nkey]);
  printf("]");
}

void
print_tree(NODE *node)
{
  if (!node) return;
  print_tree_core(node);
  printf("\n"); fflush(stdout);
}

void
print_tree_debug(NODE *node, const uint line)
{
  printf("%d: ", line);
  print_tree(node);
}

void
print_tree_debug_2(NODE *node, const uint line, const char *func)
{
  printf("%d %s: ", line, func);
  print_tree(node);
}

NODE *
alloc_leaf(NODE *parent)
{
  NODE *node;
  if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
  node->isLeaf = true;
  node->parent = parent;
  node->nkey = 0;

  return node;
}

NODE *
alloc_internal(NODE *parent)
{
  NODE *node;
  if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
  node->isLeaf = false;
  node->parent = parent;
  node->nkey = 0;

  return node;
}

NODE *
alloc_root(NODE *left, int rs_key, NODE *right)
{
  NODE *node;

  if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
  node->parent = NULL;
  node->isLeaf = false;
  node->key[0] = rs_key;
  node->child[0] = left;
  node->child[1] = right;
  node->nkey = 1;

  return node;
}

NODE *
find_leaf(NODE *node, int key)
{
  int kid;

  if (node->isLeaf) return node;
  for (kid = 0; kid < node->nkey; kid++) {
    if (key < node->key[kid]) break;
  }

  return find_leaf(node->child[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
  int i;
  if (key < leaf->key[0]) {
    for (i = leaf->nkey; i > 0; i--) {
      leaf->child[i] = leaf->child[i-1] ;
      leaf->key[i] = leaf->key[i-1] ;
    } 
    leaf->key[0] = key;
    leaf->child[0] = (NODE *)data;
  }
  else {
    for (i = 0; i < leaf->nkey; i++) {
      if (key < leaf->key[i]) break;
    }
    for (int j = leaf->nkey; j > i; j--) {		
      leaf->child[j] = leaf->child[j-1] ;
      leaf->key[j] = leaf->key[j-1] ;
    } 
    leaf->key[i] = key;
    leaf->child[i] = (NODE *)data;
  }
  leaf->nkey++;

  return leaf;
}

void
insert_in_temp(TEMP *temp, int key, void *ptr)
{
  int i;
  if (key < temp->key[0]) {
    for (i = temp->nkey; i > 0; i--) {
      temp->child[i] = temp->child[i-1] ;
      temp->key[i] = temp->key[i-1] ;
    } 
    temp->key[0] = key;
    temp->child[0] = (NODE *)ptr;
  }
  else {
    for (i = 0; i < temp->nkey; i++) {
      if (key < temp->key[i]) break;
    }
    for (int j = temp->nkey; j > i; j--) {		
      temp->child[j] = temp->child[j-1] ;
      temp->key[j] = temp->key[j-1] ;
    } 
    temp->key[i] = key;
    temp->child[i] = (NODE *)ptr;
  }

  temp->nkey++;
}

void
erase_entries(NODE *node)
{
  for (int i = 0; i < N-1; i++) node->key[i] = 0;
  for (int i = 0; i < N; i++) node->child[i] = NULL;
  node->nkey = 0;
}

void
copy_from_temp_to_left(TEMP temp, NODE *left)
{
  for (int i = 0; i < (int)ceil(N/2); i++) {
    left->key[i] = temp.key[i];
    left->child[i] = temp.child[i];
    left->nkey++;
  }
}

void
copy_from_temp_to_right(TEMP temp, NODE *right)
{
  for (int i = (int)ceil(N/2); i < N; i++) {
    right->key[i - (int)ceil(N/2)] = temp.key[i];
    right->child[i - (int)ceil(N/2)] = temp.child[i];
    right->nkey++;
  }
}

void
copy_from_temp_to_left_parent(TEMP *temp, NODE *left)
{
  for (int i = 0; i < (int)ceil((N+1)/2); i++) {
    left->key[i] = temp->key[i];
    left->child[i] = temp->child[i];
    left->nkey++;
  }
  left->child[(int)ceil((N+1)/2)] = temp->child[(int)ceil((N+1)/2)];	
}

void
copy_from_temp_to_right_parent(TEMP *temp, NODE *right)
{
  int id;

  for (id = ((int)ceil((N+1)/2) + 1); id < N; id++) {
    right->child[id - ((int)ceil((N+1)/2) + 1)] = temp->child[id];
    right->key[id - ((int)ceil((N+1)/2) + 1)] = temp->key[id];
    right->nkey++;
  }
  right->child[id - ((int)ceil((N+1)/2) + 1)] = temp->child[id];	

  for (int i = 0; i < right->nkey+1; i++) right->child[i]->parent = right;
}

void
copy_from_left_to_temp(TEMP *temp, NODE *left)
{
  int i;
  bzero(temp, sizeof(TEMP));
  for (i = 0; i < (N-1); i++) {
    temp->child[i] = left->child[i];
    temp->key[i] = left->key[i];
  } temp->nkey = N-1;
  temp->child[i] = left->child[i];	
}

void
insert_after_left_child(NODE *parent, NODE *left_child, int rs_key, NODE *right_child)
{
  int lcid = 0;
  int rcid = 0; // right_child_id
  int i;

  for (i = 0; i < parent->nkey+1; i++) {
    if (parent->child[i] == left_child) {
      lcid = i; // left_child_id
      rcid = lcid+1; break; 
    }
  } 
  assert(i != parent->nkey+1);

  for (i = parent->nkey+1; i > rcid; i--) parent->child[i] = parent->child[i-1];		
  for (i = parent->nkey; i > lcid; i--) parent->key[i] = parent->key[i-1];

  parent->key[lcid] = rs_key;
  parent->child[rcid] = right_child;
  parent->nkey++;
}

void
insert_temp_after_left_child(TEMP *temp, NODE *left_child, int rs_key, NODE *right_child)
{
  int lcid = 0;
  int rcid = 0; // right_child_id
  int i;

  for (i = 0; i < temp->nkey+1; i++) {
    if (temp->child[i] == left_child) {
      lcid = i; // left_child_id
      rcid = lcid+1; break; 
    }
  } assert(i != temp->nkey+1);

  for (i = temp->nkey+1; i > rcid; i--) temp->child[i] = temp->child[i-1];		
  for (i = temp->nkey; i > lcid; i--) temp->key[i] = temp->key[i-1];

  temp->key[lcid] = rs_key;
  temp->child[rcid] = right_child;
  temp->nkey++;
}

void
print_temp(TEMP t)
{
  int i;

  for (i = 0; i < t.nkey; i++) {
    printf("[%p]", t.child[i]);		
    printf("%d", t.key[i]);
  }
  printf("[%p]\n", t.child[i]);		
}

void
insert_in_parent(NODE *left_child, int rs_key, NODE *right_child)
{
  NODE *left_parent;
  NODE *right_parent;

  if (left_child == Root) {
    Root = alloc_root(left_child, rs_key, right_child);
    left_child->parent = right_child->parent = Root;
    return;
  }
  left_parent = left_child->parent;
  if (left_parent->nkey < N-1) {
    insert_after_left_child(left_parent, left_child, rs_key, right_child);
  }
  else {// split
    TEMP temp;
    copy_from_left_to_temp(&temp, left_parent);
    insert_temp_after_left_child(&temp, left_child, rs_key, right_child);
		
    erase_entries(left_parent);	
    right_parent = alloc_internal(left_parent->parent);
    copy_from_temp_to_left_parent(&temp, left_parent);
    int rs_key_parent = temp.key[(int)ceil(N/2)]; 
    copy_from_temp_to_right_parent(&temp, right_parent);
    insert_in_parent(left_parent, rs_key_parent, right_parent);
  }
}

void 
insert(int key, DATA *data)
{
  NODE *leaf;

  if (Root == NULL) {
    leaf = alloc_leaf(NULL);
    Root = leaf;
  }
  else {
    leaf = find_leaf(Root, key);
  }

  if (leaf->nkey < (N-1)) {
    insert_in_leaf(leaf, key, data);
  }
  else { // split
    NODE *left = leaf;
    NODE *right = alloc_leaf(leaf->parent);
    TEMP temp;

    copy_from_left_to_temp(&temp, left);
    insert_in_temp(&temp, key, data);

    right->child[N-1] = left->child[N-1];	
    left->child[N-1] = right;
    erase_entries(left);
    copy_from_temp_to_left(temp, left);
    copy_from_temp_to_right(temp, right);
    int rs_key = right->key[0]; // right smallest key
    insert_in_parent(left, rs_key, right);
  }
}

int 
find_key(const int key, NODE *node)
{
  int i;
  
  for (i = 0; i < node->nkey; i++) {
    if (node->key[i] == key) break;
  }
  if (i == node->nkey) {
    std::cout << "Key not found: " << key << std::endl;
    ERR;
  }

  return i;
}

int
get_me_in_parent(NODE *node)
{
  NODE *parent = node->parent;
  int myid = -1;

  assert(node != Root);
  for (int i = 0; i < parent->nkey+1; i++) {
    if (parent->child[i] == node) {
      myid = i; break;
    }
  } assert(myid != -1);

  return myid;
}

bool
can_coalesce_prev(NODE *node)
{
  // Coalesce?
  NODE *parent = node->parent;
  int myid = get_me_in_parent(node);

  if (myid != 0) { // check previous
    if (is_leaf(node)) {
      if ((parent->child[myid-1]->nkey + node->nkey) < (N)) {
        return true;
      }
    }
    else { // internal
      if ((parent->child[myid-1]->nkey + node->nkey) < (N-1)) {
        return true;
      }
    }
  } 

  return false;
}

bool
can_coalesce_next(NODE *node)
{
  // Coalesce?
  NODE *parent = node->parent;
  int myid = get_me_in_parent(node);

  if (myid < parent->nkey) {
    if (is_leaf(node)) {
      if (parent->child[myid + 1]->nkey + node->nkey < N) {
        return true;
      }
    }
    else { // internal
      if ((parent->child[myid + 1]->nkey + node->nkey) < (N-1)) {
        return true;
      }
    }
  }

  return false;
}

bool
can_move_from_prev(NODE *node)
{
  NODE *parent = node->parent;
  int myid = get_me_in_parent(node);

  if (myid != 0) { // check previous
    if (parent->child[myid - 1]->nkey - 1 >= ((int)ceil(N/2) - 1)) {
      return true;
    }
  } 

  return false;
}

bool
can_move_from_next(NODE *node)
{
  NODE *parent = node->parent;
  int myid = get_me_in_parent(node);

  if (myid < parent->nkey) {
    if (parent->child[myid+1]->nkey-1 >= ((int)ceil(N/2)-1)) {
      return true;
    }
  }

  return false;
}

NODE *
get_prev_node(NODE *node)
{
  NODE *prev;
  int myid = get_me_in_parent(node);
  assert(myid >= 0);
  prev = node->parent->child[myid-1];

  return prev;
}

NODE *
get_next_node(NODE *node)
{
  NODE *next;
  int myid = get_me_in_parent(node);
  assert(myid <= node->parent->nkey);
  next = node->parent->child[myid + 1];
  return next;
}

void
insert_pair(int key, NODE *ptr, NODE *dst)
{
  int pos;

  // Find 
  for (pos = 0; pos < dst->nkey; pos++) {
    if (key < dst->key[pos]) break;
  } 

  // Shift
  for (int j = dst->nkey; j > pos; j--) {		
    dst->child[j] = dst->child[j-1] ;
    dst->key[j] = dst->key[j-1] ;
  } 

  // Insert
  dst->key[pos] = key;
  dst->child[pos+1] = ptr;
  dst->nkey++;
}

void
erase_this_node(NODE *node)
{
  free(node);
}

NODE 
get_tmp_4_del(int newkey, NODE *node)
{
  NODE tmp;

  tmp.key[0] = newkey;
  for (int i = 0; i < node->nkey; i++) {
    tmp.key[i + 1] = node->key[i];
  }
  for (int i = 0; i < node->nkey + 1; i++) {
    tmp.child[i] = node->child[i];
  }
  tmp.nkey = node->nkey + 1;

  return tmp;
}

int
find_smallest(NODE *node)
{
  while (is_leaf(node) == false) {
    node = node->child[0];
  }
  return node->key[0];
}

void
exec_coalesce(NODE *node)
{
  if (is_internal(node)) { 
    // Shift and input newkey;
    int newkey = find_smallest(node);
    
    // Move
    NODE *prev = get_prev_node(node);
    NODE tmp = get_tmp_4_del(newkey, node);
    
    for (int i = 0; i < tmp.nkey; i++) {
      tmp.child[i]->parent = prev; // Reset parent
      prev->key[prev->nkey] = tmp.key[i];
      prev->child[prev->nkey + 1] = tmp.child[i];
      prev->nkey++;
    } 
  }
  else if (is_leaf(node)) {
		/* Write your code here */
  } else ERR;

  erase_this_node(node);
}

void
delete_head(NODE *node)
{
  NODE *target_ptr = node->child[0];

  // key
  for (int i = 0; i < node->nkey-1; i++) {
    node->key[i] = node->key[i+1];
  }

  // ptr
  for (int i = 0; i < node->nkey; i++) {
    node->child[i] = node->child[i+1];
  }

  node->nkey--;

  free(target_ptr);
}

// delete key & ptr from "NODE *node"
void
delete_pair(const int key, NODE *node)
{
  int target = find_key(key, node);

  // key
  for (int i = target; i < node->nkey-1; i++) {
    node->key[i] = node->key[i+1];
  }

  // ptr
  for (int i = target + 1; i < node->nkey; i++) {
    node->child[i] = node->child[i+1];
  }

  node->nkey--;
}

int 
get_incoming_key(NODE *node)
{
  NODE *parent = node->parent;
  int pos = -1;
  for (int i = 0; i < parent->nkey+1; i++) {
    if (parent->child[i] == node) {
      pos = i; break;
    }
  } assert(pos != -1);

  return parent->key[pos - 1];
}

NODE *
get_coalesce_node(NODE *node)
{
  if (can_coalesce_next(node)) {
    return get_next_node(node);
  }
  else if (can_coalesce_prev(node)) {
    return node;
  }
  return NULL;
}

void
move_from_prev(NODE *cur)
{
  if (is_internal(cur)) {
    NODE *prev = get_prev_node(cur);
    
    int new_key = cur->parent->key[get_me_in_parent(cur)-1];
    NODE* new_child = prev->child[prev->nkey];
    int new_parent_key = prev->key[prev->nkey - 1];
    
    new_child->parent = cur;
    
    // add new ptr and key to node
    for (int i = cur->nkey; i > 0; i--) {
      cur->key[i] = cur->key[i-1];
    } 
    for (int i = cur->nkey+1; i > 0; i--) {
      cur->child[i] = cur->child[i-1];
    } 
    
    cur->child[0] = new_child;
    cur->key[0] = new_key;
    cur->nkey++;
    
    // delete from prev
    prev->nkey--;
    cur->parent->key[get_me_in_parent(cur) - 1] = new_parent_key;
  }
  else { // leaf
    NODE *prev = get_prev_node(cur);
    int new_key = prev->key[prev->nkey-1];
    NODE* new_child = prev->child[prev->nkey];
    int new_parent_key = new_key;
    
    insert_pair(new_key, new_child, cur);
    delete_pair(new_key, prev);
    cur->parent->key[get_me_in_parent(cur) - 1] = new_parent_key;
  }
}

void
move_from_next(NODE *cur)
{
  if (is_internal(cur)) {
    NODE *next = get_next_node(cur);
    NODE* new_child = next->child[0];
    int new_key = find_smallest(next);
    int new_parent_key = next->key[0];

    // this node
    new_child->parent = cur;
    cur->key[cur->nkey] = new_key;
    cur->child[cur->nkey + 1] = new_child;
    cur->nkey++;
    
    // next node
    for (int i = 0; i < next->nkey - 1; i++) {
      next->key[i] = next->key[i + 1];
    }
    for (int i = 0; i < next->nkey; i++) {
      next->child[i] = next->child[i + 1];
    }
    next->nkey--;
    cur->parent->key[get_me_in_parent(cur)] = new_parent_key;
  }
  else if (is_leaf(cur)){
    NODE *next = get_next_node(cur);
    int new_key = next->key[0];
    int new_parent_key = next->key[1];
    NODE* new_child = next->child[0];

    insert_pair(new_key, new_child, cur);
    delete_pair(new_key, next);
    cur->parent->key[get_me_in_parent(cur)] = new_parent_key;
  } else ERR;
}

void
exec_redistribute(NODE *cur)
{
  if (can_move_from_next(cur)) {
    move_from_next(cur);
  }
  else if (can_move_from_prev(cur)) {
    move_from_prev(cur);
  }
  else { // never happen
    ERR;
  }
}

void
delete_entry(NODE *node, const int key)
{
  if (node == Root && node->nkey == 1) {
    Root = node->child[0];
    if (Root) Root->parent = NULL;
    node->nkey = 0;
    free(node);
    return;
  }

  delete_pair(key, node);
  if (node == Root) return;

  if (node->nkey < (int)ceil(N/2)) {
    NODE *right_hand_side = get_coalesce_node(node);
    if (right_hand_side) { //Coalesce?      
      int key = get_incoming_key(right_hand_side);
      exec_coalesce(right_hand_side);
      delete_entry(right_hand_side->parent, key);
    }
    else { // Re-distribute!
      exec_redistribute(node);
    }
  }
}

void 
do_delete(int key)
{
  NODE *node = find_leaf(Root, key);
  delete_entry(node, key);
}

void
init_root(void)
{
  Root = NULL;
}

void
update_core(const int key, DATA *data)
{
  NODE *n = find_leaf(Root, key);

  for (int i = 0; i < n->nkey; i++) {
    if (n->key[i] == key) {
      n->child[i] = (NODE *)data;
      return;
    }
  }
  cout << "Key not found: " << key << endl;
}

void
search_core(const int key)
{
  NODE *n = find_leaf(Root, key);

  for (int i = 0; i < n->nkey; i++) {
    if (n->key[i] == key) return;
  }
  cout << "Key not found: " << key << endl;
}

void 
search_single(void)
{
  for (int i = 0; i < (int)Key_vector.size(); i++) {
    search_core(Key_vector[i]);
  }
}

int 
get_key()
{
  int key;

  std::cout << "Key: ";
  std::cin >> key;

  return key;
}

void
interactive(void)
{
  char buf[BUFSIZ];
  int key;

  printf("> ");
  if (scanf("%s %d", buf, &key) == -1) ERR;
  if (buf[0] == 'd') { // delete
    do_delete(key);
  }
  else if (buf[0] == 'i') { // insert
    insert(key, NULL);    
  } 
  else if (buf[0] == 'u') { // update
    update_core(key, NULL);
  }
  else if (buf[0] == 's') { // search
    search_core(key);
  }
}

int
main(int argc, char *argv[])
{
  //Size_data = 100;
  if (argc == 2) Size_data = atoi(argv[1]);

  init_vector();
  init_root();

  printf("----Insert-----\n");
  for (int i = 0; i < Size_data; i++) {
    //printf("[ins=%3d] ", Key_vector[i]);
    insert(Key_vector[i], NULL);
  }
  
  printf("----Search-----\n");
  search_single();

  printf("----Delete-----\n");
  for (int i = 0; i < Size_data; i++) {
    //printf("[del=%3d] ", Key_vector[i]);
    do_delete(Key_vector[i]);
  }

  return 0;
}
