#include "BoxLayout.h"
#include "Size.h"
#include "WidgetContainer.h"


namespace phui {

    BoxLayout::BoxLayout(Type type) {
        _type = type;
        _margin = Insets(10, 10, 10, 10);
        _spacing = 10;
    }

    void BoxLayout::layout(WidgetContainer* container) {
        if (container->getNumChildren() == 0) {
            return;
        }        
    
        if (_type == VERTICAL) {
            Size s = container->getSize();
            int top    = _margin.getTop();
            int bottom = _margin.getBottom();
            
            int total_height = s.getHeight() - top - bottom + _spacing;
            int widget_height = total_height / container->getNumChildren();
            
            for (size_t i = 0; i < container->getNumChildren(); ++i) {
                Rect r;
                r.mX      = _margin.getLeft();
                r.mY      = top + i * widget_height;
                r.mWidth  = s.getWidth() - r.mX - _margin.getRight();
                r.mHeight = widget_height - _spacing;
                
                container->getChild(i)->setPositionAndSize(r);
            }
        }
    }

}