/* -*- Mode: C++; tab-width: 3; indent-tabs-mode: nil c-basic-offset: 3 -*- */
// vim:cindent:ts=3:sw=3:et:tw=80:sta:
/***************************************************************** phui-cpr beg
 *
 * phui - flexible user interface subsystem
 * phui is (C) Copyright 2002 by
 *      Chad Austin, Josh Carlson, Johnathan Gurley,
 *      Ben Scott, Levi Van Oort
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile: ListSelectionListener.h,v $
 * Date modified: $Date: 2003-07-22 03:24:31 $
 * Version:       $Revision: 1.1 $
 * -----------------------------------------------------------------
 *
 ************************************************************** phui-cpr-end */
#ifndef PHUI_LIST_SELECTION_LISTENER_H
#define PHUI_LIST_SELECTION_LISTENER_H

#include "Widget.h"

namespace phui
{
   /**
    * Describes an event describing a selection within a list.
    */
   class ListSelectionEvent
   {
   public:
      /**
       * Creates a new list selection event using the given source widget.
       *
       * @param src
       */
      ListSelectionEvent(WidgetPtr src, int selectedIndex)
         : mSource(src), mSelectedIndex(selectedIndex)
      {}

      /**
       * Gets the source widget that fired this event.
       *
       * @return  the source widget
       */
      WidgetPtr getSource() const
      {
         return mSource;
      }

      /**
       * Gets the index of the selected item.
       *
       * @return  the index of the selected item.
       */
      int getSelectedIndex() const
      {
         return mSelectedIndex;
      }

   private:
      /// The source of the event.
      WidgetPtr mSource;

      /// The selected index.
      int mSelectedIndex;
   };

   /**
    * Interface to a class that wishes to receive list selection events from a widgets.
    */
   class ListSelectionListener : public RefCounted
   {
   public:
      virtual ~ListSelectionListener() {}

      /**
       * Notifies this listener that the source object has fired the given
       * list selection event.
       *
       * @param evt  the generated event
       */
      virtual void onListSelection(const ListSelectionEvent& evt) = 0;
   };

   typedef RefPtr<ListSelectionListener> ListSelectionListenerPtr;
}

#endif