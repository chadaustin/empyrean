#ifndef PYR_TOOL_H
#define PYR_TOOL_H

#include <map>
#include <string>

namespace pyr {

    class MainFrame;
    class MapView;
    class Map;
    class CommandReciever;

    struct ToolEvent {
        CommandReciever* cmd;
        float x;
        float y;
        bool leftButton;
        bool rightButton;
        bool middleButton;
        bool shift;
        bool ctrl;
        bool alt;
    };

    struct GridEvent {
        CommandReciever* cmd;
        std::string name;
        std::string value;
    };

    /**
     * Superclass for all editor tools.
     * This is *almost* an interface; it also includes a reference to the MapView that owns the tool,
     * and allows tools to gain a const MapView* pointing to it.
     *
     * This is necessary for tools to be able to render things when active, and other such things.
     * the constness *should* keep silly people from altering the map without using a Command.
     * (shame on you for trying)
     */
    class Tool {
    public:
        virtual ~Tool() { }
        
        virtual bool onMouseMove(ToolEvent&) { return false; }
        virtual bool onLeftDown(ToolEvent&)  { return false; }
        virtual bool onLeftUp(ToolEvent&)    { return false; }
        virtual bool onPropertiesChanged(GridEvent&) { return false; }

        virtual void onRender(){}

    protected:
        static MainFrame* getMainFrame();
        static MapView* getMapView();
        static const Map* getMap();

        void setPropertiesGrid(std::map<std::string, std::string>& properties);
    };

}


#endif
