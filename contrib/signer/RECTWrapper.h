/* RECTWrapper.h */

#ifndef RECTWRAPPER_H
#define RECTWRAPPER_H

/*
Thin wrapper around GDI's RECT, mainly to allow what would otherwise have to
be a ton of "OffsetRect(&rect, 1, 2);"-type calls to be more easily written.
Also has a few gimmes like width() and height().  Note this is derived from
GDI's RECT *struct*, so that they're interchangable, and is not a class proper.
Not a general-purpose Rectangle class, not intended to be a general-purpose Rectangle class.
*/

struct RECTWrapper : public RECT
{
   // Get interesting facts about the RECT/RECTWrapper
   int width() const  { return right - left; };
   int height() const { return bottom - top; };

   // Return the center point of the rect.
   POINT center() const
      {
      POINT retval;

      retval.x = (left + right)/2;
      retval.y = (top + bottom)/2;

      return retval;
      }

   // Do interesting things to the RECT/RECTWrapper
   RECTWrapper& operator=(const RECT& r)
      {
      right  = r.right;
      left   = r.left;
      top    = r.top;
      bottom = r.bottom;

      return *this;
      }

   // Move the whole rect by [x,y].
   // Windows refers to this as offsetting.
   void move(int x, int y) { OffsetRect(this, x, y); }
};

#endif
