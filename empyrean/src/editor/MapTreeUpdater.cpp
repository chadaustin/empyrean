#include "MapTreeUpdater.h"
#include "MapElement.h"
#include "MapTree.h"

namespace pyr {

    MapTreeUpdater::MapTreeUpdater(wxTreeCtrl* treeView, const wxTreeItemId& root)
        : _treeView(treeView)
    {
        _parentId.push(root);

        _treeView->DeleteChildren(root);
    }

    void MapTreeUpdater::visitGeometry(GeometryElement* e) {
        _treeView->AppendItem(_parentId.top(), "Geometry", -1, -1, new TreeItemData(e));
    }

    void MapTreeUpdater::visitGroup(GroupElement* e) {
        _parentId.push(
            _treeView->AppendItem(_parentId.top(), "Group", -1, -1, new TreeItemData(e))
            );

        for (size_t i = 0; i < e->children.size(); i++) {
            e->children[i]->handleVisitor(*this);
        }

        _parentId.pop();
    }

}
