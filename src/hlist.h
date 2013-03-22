/*
 * Alper Akcan - 03.10.2009
 */

#if !defined(HLIST_H)
#define HLIST_H

struct hlist {
	struct hlist *next;
	struct hlist *prev;
};

#define HLIST_HEAD_INIT(name) { &(name), &(name) }

#define hoffsetof_(type, member) ((size_t) &((type *) 0)->member)

#define hcontainerof_(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - hoffsetof_(type, member)); })

#define hlist_entry(ptr, type, member) \
	hcontainerof_(ptr, type, member)

#define hlist_first_entry(ptr, type, member) \
	hlist_entry((ptr)->next, type, member)

#define hlist_last_entry(ptr, type, member) \
	hlist_entry((ptr)->prev, type, member)

#define hlist_next_entry(ptr, list_ptr, type, member) \
	((ptr)->next == list_ptr) ? hlist_entry((list_ptr)->next, type, member) : hlist_entry((ptr)->next, type, member)

#define hlist_prev_entry(ptr, list_ptr, type, member) \
	((ptr)->prev == list_ptr) ? hlist_entry((list_ptr)->prev, type, member) : hlist_entry((ptr)->prev, type, member)

#define hlist_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define hlist_for_each_entry(pos, head, member)				\
	for (pos = hlist_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = hlist_entry(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_prev(pos, head, member)				\
	for (pos = hlist_entry((head)->prev, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = hlist_entry(pos->member.prev, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)			\
	for (pos = hlist_entry((head)->next, typeof(*pos), member),	\
	     n = hlist_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = hlist_entry(n->member.next, typeof(*n), member))

#define hlist_for_each_entry_safe_prev(pos, n, head, member)			\
	for (pos = hlist_entry((head)->prev, typeof(*pos), member),	\
	     n = hlist_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = hlist_entry(n->member.prev, typeof(*n), member))

static inline int hlist_count (struct hlist *head)
{
	int count;
	struct hlist *entry;
	count = 0;
	hlist_for_each(entry, head) {
		count++;
	}
	return count;
}

static inline void hlist_add_actual (struct hlist *elem, struct hlist *prev, struct hlist *next)
{
	next->prev = elem;
	elem->next = next;
	elem->prev = prev;
	prev->next = elem;
}

static inline void hlist_del_actual (struct hlist *prev, struct hlist *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void hlist_add_tail (struct hlist *elem, struct hlist *head)
{
	hlist_add_actual(elem, head->prev, head);
}

static inline void hlist_add (struct hlist *elem, struct hlist *head)
{
	hlist_add_actual(elem, head, head->next);
}

static inline void hlist_concat (struct hlist *dest, struct hlist *src)
{
	dest->prev->next = src->next;
	src->next->prev = dest->prev;
	src->prev->next = dest;
	dest->prev = src->prev;
	src->prev = src;
	src->next = src;
}

static inline void hlist_del (struct hlist *entry)
{
	hlist_del_actual(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline int hlist_is_first (struct hlist *list, struct hlist *head)
{
	return head->next == list;
}

static inline int hlist_is_last (struct hlist *list, struct hlist *head)
{
	return list->next == head;
}

static inline void hlist_init (struct hlist *head)
{
	head->next = head;
	head->prev = head;
}

static inline int hlist_empty (struct hlist *head)
{
	return head->next == head;
}

#endif
