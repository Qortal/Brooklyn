#ifndef WINDOW_TREE_H
# define WINDOW_TREE_H
# include <e.h>

/* XXX: First two have to be 0 and 1 because I use them with modulo. */
typedef enum
{
   TILING_SPLIT_HORIZONTAL = 0,
   TILING_SPLIT_VERTICAL = 1,
   TILING_SPLIT_FLOAT,
   TILING_SPLIT_LAST
} Tiling_Split_Type;

typedef struct _Window_Tree Window_Tree;

/* Root is level 0. Each node's split type is (level % 2). */
struct _Window_Tree
{
   EINA_INLIST;
   Window_Tree *parent;
   /* FIXME: client is falid iff children is null. Sholud enforce it. */
   Eina_Inlist *children; /* Window_Tree * type */
   E_Client    *client;
   struct {
      int x, y, w, h;
   } space;
   double       weight;
};

# define TILING_WINDOW_TREE_EDGE_LEFT   (1 << 0)
# define TILING_WINDOW_TREE_EDGE_RIGHT  (1 << 1)
# define TILING_WINDOW_TREE_EDGE_TOP    (1 << 2)
# define TILING_WINDOW_TREE_EDGE_BOTTOM (1 << 3)

int          tiling_window_tree_edges_get(Window_Tree *node);

void         tiling_window_tree_free(Window_Tree *root);
void         tiling_window_tree_walk(Window_Tree *root, void (*func)(void *));

/**
 * Insert a new client into the tree
 *
 * @param root the root node where to insert in some subtree
 * @param buddy the buddy where you want to place client to, buddy MUST be a node with a associated client OR NULL
 * @param split_type the split type you want to insert the client
 * @param before true to insert the client before the buddy false to insert it after the buddy
 *
 * @return the new root node
 */
Window_Tree *tiling_window_tree_insert(Window_Tree *root, Window_Tree *buddy,
                                       E_Client *client, Tiling_Split_Type split_type, Eina_Bool before);

Window_Tree *tiling_window_tree_remove(Window_Tree *root, Window_Tree *item);

Window_Tree *tiling_window_tree_client_find(Window_Tree *root,
                                            E_Client *client);

Eina_Bool    tiling_window_tree_apply(Window_Tree *root, Evas_Coord x, Evas_Coord y,
                                      Evas_Coord w, Evas_Coord h, Evas_Coord padding,
                                      Eina_Bool force_float);

Eina_Bool    tiling_window_tree_node_resize(Window_Tree *node, int w_dir,
                                            double w_diff, int h_dir, double h_diff);

void         tiling_window_tree_node_change_pos(Window_Tree *node, int key);

Eina_Bool    tiling_window_tree_repair(void);
#endif
