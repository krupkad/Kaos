#ifndef LIST_H
#define LIST_H

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_POISON1 ((void *)0x00100100)
#define LIST_POISON2 ((void *)0x00200200)

#define LIST_HEAD_INIT(name)) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_heaad name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_heaad *list) {
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *new, struct list_head *prev,
								struct list_head *next) {
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *list) {
	__list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *n, struct list_head *list) {
	__list_add(new, list->prev, list);
}

static inline void __list_del(struct list_head *prev, struct list_head *next) {
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *entry) {
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

static inline void list_replace(struct list_head *old, struct list_head *new) {
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void list_del_init(struct list_head *entry) {
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head *list, struct list_head *head) {
	__list_del(list->prev, list->next);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *l, struct list_head *h) {
	__list_del(l->prev, l->next);
	list_add_tail(l, h);
}

static inline int list_is_last(struct list_head *l, struct list_head *h) {
	return l->next == h;
}

static inline int list_empty(struct list_head *head) {
	return head->next == head;
}

static inline int list_empty_careful(struct list_head *head) {
	struct list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

static inline void list_rotl(struct list_head *head) {
	struct list_head *first;
	if(!list_empty(head)) {
		first = head->next;
		list_move_tail(first,head);
	}
}

#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr) - (uaddr_t)(&((type *)0)->member)))

#endif /* LIST_H */
