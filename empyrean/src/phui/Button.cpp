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
 * File:          $RCSfile: Button.cpp,v $
 * Date modified: $Date: 2003-08-05 05:00:28 $
 * Version:       $Revision: 1.2 $
 * -----------------------------------------------------------------
 *
 ************************************************************** phui-cpr-end */
#include <algorithm>
#include "OpenGL.h"
#include <gltext.h>
#include "Button.h"
#include "WidgetContainer.h"

namespace phui
{
   Button::Button()
      : mText("")
      , mButtonDown(false)
   {}

   Button::Button(const std::string& text)
      : mText(text)
      , mButtonDown(false)
   {}

   void Button::draw()
   {
      const Size& size = getSize();
      const int width = size.getWidth();
      const int height = size.getHeight();

      // draw the button background
      glColor(mButtonDown ? getForegroundColor() : getBackgroundColor());
      glBegin(GL_TRIANGLE_FAN);
      {
         glVertex2i(0,     0     );
         glVertex2i(width, 0     );
         glVertex2i(width, height);
         glVertex2i(0,     height);
      }
      glEnd();

      // draw text
      glColor(mButtonDown ? getBackgroundColor() : getForegroundColor());

      gltext::FontPtr font = getFont();
      gltext::FontRendererPtr renderer = getFontRenderer();
      float labelWidth = float(renderer->getWidth(mText.c_str()));
      float fontHeight = float(font->getAscent() + font->getDescent());

      //Lets store the Matrix so we don't piss anyone off
      glPushMatrix();

      //These checks see if the button Label fits inside the
      //button.  If not start in the lower left-hand corner of
      //the button and render the text.
      float xLoc = std::max((width  - labelWidth) / 2.0f, 0.0f);
      float yLoc = std::max((height - fontHeight) / 2.0f, 0.0f);
      glTranslatef(xLoc, height - yLoc, 0.0f);
      
      renderer->render(mText.c_str());

      //Lets restore the Matrix
      glPopMatrix();

      if (hasFocus())
      {
         glBegin(GL_LINE_LOOP);
         glVertex2i(0,     0     );
         glVertex2i(width, 0     );
         glVertex2i(width, height);
         glVertex2i(0,     height);
         glEnd();
      }
   }

   void Button::setText(const std::string& text)
   {
      mText = text;
   }

   const std::string& Button::getText() const
   {
      return mText;
   }

   void Button::onMouseDown(InputButton button, const Point& p)
   {
      if (button == BUTTON_LEFT)
      {
         mButtonDown = true;
      }
   }

   void Button::onMouseUp(InputButton button, const Point& p)
   {
      if (mButtonDown && button == BUTTON_LEFT)
      {
         mButtonDown = false;

         // Only fire button pressed event if the mouse was released
         // inside this button.
         if (contains(p))
         {
            fireActionEvent();
         }
      }
   }

   void Button::addActionListener(ActionListenerPtr listener)
   {
      mListeners.push_back(listener);
   }

   void Button::removeActionListener(ActionListenerPtr listener)
   {
      ListenerIter itr =
         std::find(mListeners.begin(), mListeners.end(), listener);
      if (itr != mListeners.end())
      {
         mListeners.erase(itr);
      }
   }

   void Button::fireActionEvent()
   {
      ActionEvent evt(this);
      for(ListenerIter itr=mListeners.begin(); itr!=mListeners.end(); itr++)
      {
         (*itr)->onAction(evt);
      }
   }
}