#include "CreateMapElementCommand.h"
#include "Editor.h"
#include "MainFrame.h"
#include "Map.h"
#include "MapElement.h"
#include "MapTree.h"
#include "MapTreeUpdater.h"

namespace pyr {

    enum {
        ID_FIRST = wxID_HIGHEST,

        // Context menu items
        ID_NEW_GEO_NODE,
        ID_DESTROY_NODE,
    };

    BEGIN_EVENT_TABLE(MapTree, wxTreeCtrl)
        EVT_RIGHT_DOWN(MapTree::onRightClick)

        EVT_MENU(ID_NEW_GEO_NODE, MapTree::onNewGeoNode)
        EVT_MENU(ID_DESTROY_NODE, MapTree::onDestroyNode)
    END_EVENT_TABLE()

    MapTree::MapTree(wxWindow* parent, const wxPoint& pos, const wxSize& size)
        : wxTreeCtrl(parent, -1, pos, size, wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT)
        , _contextMenu(new wxMenu)
    {
        _contextMenu->Append(ID_NEW_GEO_NODE, "New Geometry Node");
    }

    MapTree::~MapTree() {
        delete _contextMenu;
    }

    void MapTree::update(MapPtr map) {
        MapTreeUpdater updater(this, GetRootItem());
        DeleteChildren(GetRootItem());
        map->getRoot()->handleVisitor(updater);
    }

    MapElement* MapTree::getSelection() const {
        // GetSelection returns a wxTreeItemId, getSelection returns the map element.  GOOD NAME
        TreeItemData* tid = static_cast<TreeItemData*>(GetItemData(GetSelection()));

        return tid ? tid->element.get() : 0;
    }

    void MapTree::onRightClick(wxMouseEvent& event) {
        int flags;
        wxTreeItemId id = HitTest(event.GetPosition(), flags);

        if (id) {
            PopupMenu(_contextMenu, event.GetPosition());
        }
    }

    void MapTree::onNewGeoNode(wxCommandEvent&) {
        wxTreeItemId id = GetSelection();
        if (!id.IsOk()) return;

        TreeItemData* data = static_cast<TreeItemData*>(GetItemData(id));
        wxASSERT(data != 0);

        // Maybe dynamic_cast isn't such a good idea.  I dunno.
        GroupElement* group = dynamic_cast<GroupElement*>(data->element.get());
        if (id.IsOk() && group) {
            GeometryElement* element = new GeometryElement();

            // Don't think I like this.
            Editor::getMainFrame()->handleCommand(new CreateMapElementCommand(element, group));
            update(Editor::getMainFrame()->getMap());
        }
    }

    void MapTree::onDestroyNode(wxCommandEvent&) {
    }

}
