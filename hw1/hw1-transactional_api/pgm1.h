#define LAYOUT_NAME "my_persistent_hash"
#define FILE_NAME "myhash"
#define MAX_VAL_LEN 64000

struct my_hash {
	uint64_t key;
	char val[MAX_VAL_LEN];
};

/*struct my_root {
	uint64_t key;
	char val[MAX_VAL_LEN];
	size_t lenval;
};


void node_read_init() {
	TOID(struct my_root) root = POBJ_ROOT(pop, struct root);

	current_node = POBJ_LIST_FIRST(&D_RO(root)->head);
}

void node_read_next() {
	current_node = POBJ_LIST_NEXT(current_node, nodes);
}

void node_read_prev() {
	current_node = POBJ_LIST_PREV(current_node, nodes);
}

void note_print(const TOID(struct node) node) {
	printf("Created at %s: \n %s\n",
	       ctime(D_RO(node)->date_created),
	       D_RO(node)->msg);
}

int node_construct(PMEMobjpool *pop, void *ptr, void *arg) {
	struct node *n = ptr;
	char *msg = arg;

	pmemobj_memcpy_persist(pop, n->msg, msg, strlen(msg));

	time(&n->date_created);
	pmemobj_persist(pop, &n->date_created, sizeof time_t);

	return 0;
}

void create_node(char *msg) {
	TOID(struct my_root) root = POBJ_ROOT(pop, struct root);

	size_t nlen = strlen(msg);
	POBJ_LIST_INSERT_NEW_HEAD(pop, &D_RW(root)->head, nodes,
				  sizeof(struct node) + nlen,
				  node_construct, msg);
}
*/
