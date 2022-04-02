#include "e.h"

#include "window_tree.h"
#include "e_mod_tiling.h"

void tiling_window_tree_dump(Window_Tree *root, int level);

void
tiling_window_tree_walk(Window_Tree *root, void (*func)(void *))
{
   Eina_Inlist *itr_safe;
   Window_Tree *itr;

   if (!root)
     return;

   EINA_INLIST_FOREACH_SAFE(root->children, itr_safe, itr)
     {
        tiling_window_tree_walk(itr, func);
     }
   func(root);
}

void
tiling_window_tree_free(Window_Tree *root)
{
   tiling_window_tree_walk(root, free);
}

static void
_tiling_window_tree_split_add(Window_Tree *parent, Window_Tree *new_node, Eina_Bool append)
{
   /* Make a new node for the parent client and split the weights in half. */
   Window_Tree *new_parent_client = calloc(1, sizeof(*new_node));

   new_node->parent = parent;
   new_parent_client->parent = parent;
   new_parent_client->client = parent->client;
   parent->client = NULL;
   new_parent_client->weight = 0.5;
   new_node->weight = 0.5;

   parent->children = eina_inlist_append(parent->children, EINA_INLIST_GET(new_parent_client));

   if (append)
     parent->children = eina_inlist_append(parent->children, EINA_INLIST_GET(new_node));
   else
     parent->children = eina_inlist_prepend(parent->children, EINA_INLIST_GET(new_node));

}
static void
_tiling_window_tree_parent_add(Window_Tree *parent, Window_Tree *new_node, Window_Tree *rel, Eina_Bool append)
{
   /* Adjust existing children's weights */
   Window_Tree *itr;
   int children_count = eina_inlist_count(parent->children);
   float weight = 1.0 / (children_count + 1);

   new_node->parent = parent;
   new_node->weight = weight;

   weight *= children_count;
   EINA_INLIST_FOREACH(parent->children, itr)
     {
        itr->weight *= weight;
     }
   if (append)
     {
        parent->children = eina_inlist_append_relative(parent->children,
                                                       EINA_INLIST_GET(new_node),
                                                       EINA_INLIST_GET(rel));
     }
   else
     {
        parent->children = eina_inlist_prepend_relative(parent->children,
                                                        EINA_INLIST_GET(new_node),
                                                        EINA_INLIST_GET(rel));
     }

}

static int
_tiling_window_tree_split_type_get(Window_Tree *node)
{
   int ret = 0;

   while (node->parent)
     {
        ret++;
        node = node->parent;
     }

   return ret % 2;
}

#define VERIFY_TYPE(t) if (t > TILING_SPLIT_VERTICAL || t < 0) { ERR("Invalid insert type"); return root; }

Window_Tree *
tiling_window_tree_insert(Window_Tree *root, Window_Tree *buddy,
                          E_Client *client, Tiling_Split_Type split_type, Eina_Bool before)
{
   Window_Tree *new_node;
   Window_Tree *parent;
   Tiling_Split_Type parent_split_type;

   VERIFY_TYPE(split_type)

   new_node = calloc(1, sizeof(*new_node));
   new_node->client = client;

   if (!root)
     {
        root = calloc(1, sizeof(*root));
        root->weight = 1.0;

        _tiling_window_tree_parent_add(root, new_node, NULL, !before);
     }
   else
     {
        //if there is no buddy we are going to take the last child of the root
        if (!buddy)
          {
             buddy = root;
             do
               {
                  buddy = EINA_INLIST_CONTAINER_GET(eina_inlist_last(buddy->children), Window_Tree);
               }
             while (!buddy->client);
          }
        else
          {
             //make sure this buddy has a client,
             if (!buddy->client) free(new_node);
             EINA_SAFETY_ON_TRUE_RETURN_VAL(!buddy->client, root);
          }

        parent = buddy->parent;
        parent_split_type = _tiling_window_tree_split_type_get(parent);

        if (parent_split_type == split_type)
          {
             _tiling_window_tree_parent_add(parent, new_node, buddy, !before);
          }
        else
          {
             _tiling_window_tree_split_add(buddy, new_node, !before);
          }
     }

   return root;
}

static Window_Tree *
tiling_window_tree_unref(Window_Tree *root, Window_Tree *item)
{
   if (!item->client)
     {
        ERR("Tried to unref node %p that doesn't have a client.", item);
        return NULL;
     }
   Window_Tree *parent = item->parent;
   int children_count = eina_inlist_count(item->parent->children);

   if (children_count <= 2)
     {
        Window_Tree *grand_parent = parent->parent;
        Window_Tree *item_keep = NULL;

        EINA_INLIST_FOREACH(parent->children, item_keep)
          {
             if (item_keep != item)
               break;
          }
        if (!item_keep)
          {
             parent->children =eina_inlist_remove(parent->children, EINA_INLIST_GET(item));
             return parent;
          }
        else if (!item_keep->children && (parent != root))
          {
             parent->client = item_keep->client;
             parent->children = NULL;
             return grand_parent; //we must have a grand_parent here, case the parent is not root
          }
        else
          {
             parent->children =
               eina_inlist_remove(parent->children, EINA_INLIST_GET(item));
             if (grand_parent)
               {
                  /* Update the children's parent. */
                  {
                     Eina_Inlist *itr_safe;
                     Window_Tree *itr;

                     EINA_INLIST_FOREACH_SAFE(item_keep->children, itr_safe,
                                               itr)
                       {
                          /* We are prepending to double-reverse the order. */
                          grand_parent->children =
                            eina_inlist_prepend_relative(grand_parent->children,
                                                         EINA_INLIST_GET(itr), EINA_INLIST_GET(parent));
                          itr->weight *= parent->weight;
                          itr->parent = grand_parent;
                       }

                     grand_parent->children =
                       eina_inlist_remove(grand_parent->children,
                                          EINA_INLIST_GET(parent));
                     free(parent);
                  }
                  return grand_parent;
               }
             else if (item_keep)
               {
                  /* This is fine, as this is a child of the root so we allow
                   * two levels. */
                  item_keep->weight = 1.0;
                  return item_keep->parent;
               }
          }
     }
   else
     {
        Window_Tree *itr;
        float weight = 1.0 - item->weight;

        parent->children =
          eina_inlist_remove(parent->children, EINA_INLIST_GET(item));

        /* Adjust existing children's weights */
        EINA_INLIST_FOREACH(parent->children, itr)
          {
             itr->weight /= weight;
          }
        return parent;
     }
     ERR("This is a state where we should never come to.\n");
     return NULL;
}

Window_Tree *
tiling_window_tree_remove(Window_Tree *root, Window_Tree *item)
{
   if (root == item)
     {
        free(item);
        return NULL;
     }
   else if (!item->client)
     {
        ERR("Tried deleting node %p that doesn't have a client.", item);
        return root;
     }
   tiling_window_tree_unref(root, item);
   free(item);
   if (eina_inlist_count(root->children) == 0)
     {
        //the last possible client was closed so we remove root
        free(root);
        return NULL;
     }

   return root;
}

Window_Tree *
tiling_window_tree_client_find(Window_Tree *root, E_Client *client)
{
   Window_Tree *itr;

   if (!client)
     return NULL;

   if (!root || (root->client == client))
     return root;

   EINA_INLIST_FOREACH(root->children, itr)
     {
        Window_Tree *ret;

        ret = tiling_window_tree_client_find(itr, client);
        if (ret)
          return ret;
     }

   return NULL;
}

void
_tiling_window_tree_level_apply(Window_Tree *root, Evas_Coord x, Evas_Coord y,
                                Evas_Coord w, Evas_Coord h, int level, Evas_Coord padding,
                                Eina_List **floaters)
{
   Window_Tree *itr;
   Tiling_Split_Type split_type = level % 2;
   double total_weight = 0.0;

   root->space.x = x;
   root->space.y = y;
   root->space.w = w - padding;
   root->space.h = h - padding;

   if (root->client)
     {
        if (!e_object_is_del(E_OBJECT(root->client)))
          {
             if ((root->client->icccm.min_w > (w - padding)) ||
                 (root->client->icccm.min_h > (h - padding))) {
               *floaters = eina_list_append(*floaters, root->client);
             } else {
               tiling_e_client_move_resize_extra(root->client, x, y,
                                                 w - padding, h - padding);
             }


          }
        return;
     }

   if (split_type == TILING_SPLIT_HORIZONTAL)
     {
        EINA_INLIST_FOREACH(root->children, itr)
          {
             Evas_Coord itw = w * itr->weight;

             total_weight += itr->weight;
             _tiling_window_tree_level_apply(itr, x, y, itw, h, level + 1, padding, floaters);
             x += itw;
          }
     }
   else if (split_type == TILING_SPLIT_VERTICAL)
     {
        EINA_INLIST_FOREACH(root->children, itr)
          {
             Evas_Coord ith = h * itr->weight;

             total_weight += itr->weight;
             _tiling_window_tree_level_apply(itr, x, y, w, ith, level + 1, padding, floaters);
             y += ith;
          }
     }

   /* Adjust the last item's weight in case weight < 1.0 */
   ((Window_Tree *)(void *)root->children->last)->weight += 1.0 - total_weight;
}

Eina_Bool
tiling_window_tree_apply(Window_Tree *root, Evas_Coord x, Evas_Coord y,
                         Evas_Coord w, Evas_Coord h, Evas_Coord padding,
                         Eina_Bool force_float)
{
   Eina_List *floaters = NULL;
   E_Client *ec;

   x += padding;
   y += padding;
   w -= padding;
   h -= padding;
   _tiling_window_tree_level_apply(root, x, y, w, h, 0, padding, &floaters);

   if (floaters)
     {
        // some clients failed to properly fit and maybe SHOULD float
        EINA_LIST_FREE(floaters, ec)
          {
             if (force_float) tiling_e_client_does_not_fit(ec);
          }
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Window_Tree *
_inlist_next(Window_Tree *it)
{
   return (Window_Tree *)(void *)EINA_INLIST_GET(it)->next;
}

static Window_Tree *
_inlist_prev(Window_Tree *it)
{
   return (Window_Tree *)(void *)EINA_INLIST_GET(it)->prev;
}

static Eina_Bool
_tiling_window_tree_node_resize_direction(Window_Tree *node,
                                          Window_Tree *parent, double dir_diff, int dir)
{
   double weight = 0.0;
   double weight_diff;
   Window_Tree *children_start;
   Window_Tree *itr;
   Window_Tree *(*itr_func)(Window_Tree *);

   if (dir > 0)
     {
        itr_func = _inlist_prev;
        children_start = (Window_Tree *)(void *)parent->children->last;
     }
   else
     {
        itr_func = _inlist_next;
        children_start = (Window_Tree *)(void *)parent->children;
     }

   itr = (Window_Tree *)children_start;
   while (itr != node)
     {
        weight += itr->weight;

        itr = itr_func(itr);
     }

   /* If it's at the edge, try the grandpa of the parent. */
   if (!EINA_DBL_NONZERO(weight))
     {
        if (parent->parent && parent->parent->parent)
          {
             return _tiling_window_tree_node_resize_direction(parent->parent,
                                                              parent->parent->parent, 1.0 + ((dir_diff - 1.) * node->weight),
                                                              dir);
          }
        return EINA_FALSE;
     }

   weight_diff = itr->weight;
   itr->weight *= dir_diff;
   weight_diff -= itr->weight;
   weight_diff /= weight;

   for (itr = children_start; itr != node; itr = itr_func(itr))
     {
        itr->weight += itr->weight * weight_diff;
     }

   return EINA_TRUE;
}

Eina_Bool
tiling_window_tree_node_resize(Window_Tree *node, int w_dir, double w_diff,
                               int h_dir, double h_diff)
{
   Window_Tree *parent = node->parent;
   Window_Tree *w_parent, *h_parent;
   Eina_Bool ret = EINA_FALSE;

   /* If we have no parent, means we need to be full screen anyway. */
   if (!parent)
     return EINA_FALSE;

   Window_Tree *grand_parent = parent->parent;
   Tiling_Split_Type parent_split_type =
     _tiling_window_tree_split_type_get(parent);

   /* w_diff related changes. */
   if (parent_split_type == TILING_SPLIT_HORIZONTAL)
     {
        w_parent = parent;
        h_parent = grand_parent;
     }
   else
     {
        w_parent = grand_parent;
        h_parent = parent;
     }

   if ((!eina_dbl_exact(h_diff, 1.0)) && h_parent)
     {
        Window_Tree *tmp_node = (h_parent == parent) ? node : parent;

        ret = ret ||
          _tiling_window_tree_node_resize_direction(tmp_node, h_parent,
                                                    h_diff, h_dir);
     }

   if ((!eina_dbl_exact(w_diff, 1.0)) && w_parent)
     {
        Window_Tree *tmp_node = (w_parent == parent) ? node : parent;

        ret = ret ||
          _tiling_window_tree_node_resize_direction(tmp_node, w_parent,
                                                    w_diff, w_dir);
     }

   return ret;
}

int
_tiling_window_tree_edges_get_helper(Window_Tree *node,
                                     Tiling_Split_Type split_type, Eina_Bool gave_up_this,
                                     Eina_Bool gave_up_parent)
{
   int ret =
     TILING_WINDOW_TREE_EDGE_LEFT | TILING_WINDOW_TREE_EDGE_RIGHT |
     TILING_WINDOW_TREE_EDGE_TOP | TILING_WINDOW_TREE_EDGE_BOTTOM;
   if (!node->parent)
     {
        return ret;
     }
   else if (gave_up_this && gave_up_parent)
     {
        return 0;
     }
   else if (gave_up_this)
     {
        /* Mixed the gave_up vals on purpose, we do it on every call. */
        return _tiling_window_tree_edges_get_helper(node->parent, !split_type,
                                                    gave_up_parent, gave_up_this);
     }

   if (EINA_INLIST_GET(node)->prev)
     {
        gave_up_this = EINA_TRUE;
        ret ^=
          (split_type ==
           TILING_SPLIT_HORIZONTAL) ? TILING_WINDOW_TREE_EDGE_LEFT :
          TILING_WINDOW_TREE_EDGE_TOP;
     }

   if (EINA_INLIST_GET(node)->next)
     {
        gave_up_this = EINA_TRUE;
        ret ^=
          (split_type ==
           TILING_SPLIT_HORIZONTAL) ? TILING_WINDOW_TREE_EDGE_RIGHT :
          TILING_WINDOW_TREE_EDGE_BOTTOM;
     }

   /* Mixed the gave_up vals on purpose, we do it on every call. */
   return ret & _tiling_window_tree_edges_get_helper(node->parent, !split_type,
                                                     gave_up_parent, gave_up_this);
}

int
tiling_window_tree_edges_get(Window_Tree *node)
{
   Tiling_Split_Type split_type = _tiling_window_tree_split_type_get(node);

   return _tiling_window_tree_edges_get_helper(node, !split_type, EINA_FALSE,
                                               EINA_FALSE);
}

/* Node move */
/**
 * - break would mean that the node will be breaked out in the parent node and
 *   put into the grand parent node. If there is no grand parent node a fake node will be placed there.
 *
 * - join would mean that the node will be put together with the next/previous
 *   child into a new split.
 *
 * - The eina_bool dir is the flag in which direction the node will be added ,
 *   appended or prependet, or joined with the previous child or the next child
 */
void
_tiling_window_tree_node_break_out(Window_Tree *root, Window_Tree *node, Window_Tree *par, Eina_Bool dir)
{
   Window_Tree *res, *ac; /* ac is the child of the root, that is a parent of a node */

   if (!par)
     {
        /* we have no parent, so we add a new node between this, later we do the normal break, */
        Window_Tree *rnode, *newnode, *newnode2;
        Eina_Inlist *il;

        newnode2 = calloc(1, sizeof(Window_Tree));
        newnode2->parent = root;
        newnode2->weight = 1.0;

        newnode = calloc(1, sizeof(Window_Tree));
        newnode->weight = 1.0;
        newnode->parent = newnode2;

        EINA_INLIST_FOREACH_SAFE(root->children, il, rnode)
          {
             rnode->parent = newnode;
             root->children = eina_inlist_remove(root->children, EINA_INLIST_GET(rnode));
             newnode->children = eina_inlist_append(newnode->children, EINA_INLIST_GET(rnode));
          }

        root->children = eina_inlist_append(root->children, EINA_INLIST_GET(newnode2));
        newnode2->children = eina_inlist_append(newnode2->children, EINA_INLIST_GET(newnode));
        par = newnode2;

     }

   /* search a path from the node to the par */
   ac = node;
   while (ac->parent != par)
     ac = ac->parent;

   if (dir)
     {
        res = _inlist_next(ac);
        if (res)
          {
             dir = EINA_FALSE;
          }
     }
   else
     {
        res = _inlist_prev(ac);
        if (res)
          {
             dir = EINA_TRUE;
          }
     }

   tiling_window_tree_unref(root, node);

   _tiling_window_tree_parent_add(par, node, res, dir);
}

void
_tiling_window_tree_node_join(Window_Tree *root, Window_Tree *node, Eina_Bool dir)
{
   Window_Tree *pn, *pl, *par;

   if (dir)
      pn = _inlist_next(node);
   else
      pn = _inlist_prev(node);

   if (!pn)
     {
        if (node->parent && node->parent->parent && node->parent->parent->parent)
          {
             _tiling_window_tree_node_break_out(root, node, node->parent->parent->parent, dir);
          }

        return;
     }

   par = node->parent;
   if ((eina_inlist_count(par->children) == 2) &&
      ((_inlist_next(node) && _inlist_next(node)->client) ||
       (_inlist_prev(node) && _inlist_prev(node)->client)))
      /* swap if there are just 2 simple windows*/
     {
        par->children = eina_inlist_demote(par->children, eina_inlist_first(par->children));
        return;
     }
   else
     {
        pl = tiling_window_tree_unref(root, node);
        if (pl == node->parent)
          {
             //unref has not changed the tree
             if (!pn->children)
               {
                  _tiling_window_tree_split_add(pn, node, EINA_TRUE);
               }
             else
               {
                  _tiling_window_tree_parent_add(pn, node, NULL, EINA_TRUE);
               }
          }
        else
          {
             //unref changed the position of pn in the tree, result of unref
             //will be the new parent
             _tiling_window_tree_parent_add(pl, node, NULL, EINA_TRUE);
          }
     }
}

void
tiling_window_tree_node_change_pos(Window_Tree *node, int key)
{
   if (!node->parent)
     return;

   Tiling_Split_Type parent_split_type =
     _tiling_window_tree_split_type_get(node->parent);

   Window_Tree *root = node->parent,
               *grand_parent = NULL;
   while(root->parent)
      root = root->parent;

   if (node->parent && node->parent->parent)
     grand_parent = node->parent->parent;

   switch(key)
     {
      case TILING_WINDOW_TREE_EDGE_LEFT:
         if (parent_split_type == TILING_SPLIT_HORIZONTAL)
            _tiling_window_tree_node_join(root, node, EINA_FALSE);
         else if (parent_split_type == TILING_SPLIT_VERTICAL)
            _tiling_window_tree_node_break_out(root, node, grand_parent, EINA_FALSE);
         break;
      case TILING_WINDOW_TREE_EDGE_RIGHT:
         if (parent_split_type == TILING_SPLIT_HORIZONTAL)
            _tiling_window_tree_node_join(root, node, EINA_TRUE);
         else if (parent_split_type == TILING_SPLIT_VERTICAL)
            _tiling_window_tree_node_break_out(root, node, grand_parent,EINA_TRUE);
         break;
      case TILING_WINDOW_TREE_EDGE_TOP:
         if (parent_split_type == TILING_SPLIT_HORIZONTAL)
            _tiling_window_tree_node_break_out(root, node, grand_parent, EINA_FALSE);
         else if (parent_split_type == TILING_SPLIT_VERTICAL)
            _tiling_window_tree_node_join(root, node, EINA_FALSE);
         break;
      case TILING_WINDOW_TREE_EDGE_BOTTOM:
         if (parent_split_type == TILING_SPLIT_HORIZONTAL)
            _tiling_window_tree_node_break_out(root, node, grand_parent, EINA_TRUE);
         else if (parent_split_type == TILING_SPLIT_VERTICAL)
            _tiling_window_tree_node_join(root, node, EINA_TRUE);
          break;
     }
}

/* End Node move. */

void
tiling_window_tree_dump(Window_Tree *root, int level)
{
   int i;

   if (!root)
     return;

   for (i = 0; i < level; i++)
     printf(" ");

   if (root->children)
     printf("\\-");
   else
     printf("|-");

   printf("%f (%p)\n", root->weight, root->client);

   Window_Tree *itr;

   EINA_INLIST_FOREACH(root->children, itr)
     {
        tiling_window_tree_dump(itr, level + 1);
     }
}
