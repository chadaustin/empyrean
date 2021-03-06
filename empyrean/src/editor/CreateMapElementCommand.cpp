#include "CreateMapElementCommand.h"
#include "MapTree.h"

namespace pyr {

    CreateMapElementCommand::CreateMapElementCommand(
        const MapElementPtr& element,
        const GroupElementPtr& parentElement)
    {
        _element = element;
        _parentElement = parentElement;
    }

    void CreateMapElementCommand::perform(CommandContext& context) {
        _parentElement->children.push_back(_element);
        context.mapTree->update(context.map);
    }

    void CreateMapElementCommand::undo(CommandContext& context) {
        _parentElement->children.pop_back();
        context.mapTree->update(context.map);
    }

}
