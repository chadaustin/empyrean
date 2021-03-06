#ifndef PYR_CANVAS_BLITTER_H
#define PYR_CANVAS_BLITTER_H

#include <algorithm>

// This is pretty cool stuff.  Basically, I'm using C++ templates to
// inline the pixel blending functions.  Not a lot of code doing a
// whole lot of work. :)

namespace pyr {
    struct Opaque
    {
        static inline RGBA Blend(RGBA src, RGBA)
        {
            return src;
        }
    };

    struct Matte
    {
        static inline RGBA Blend(RGBA src, RGBA dest)
        {
            return src.a?src:dest;
        }
    };

    struct Alpha
    {
        static inline RGBA Blend(RGBA src, RGBA dest)
        {
#if 1 || !defined(MSVC) || !defined(INLINE_ASM)
            // Not as fast as it could be
            if (!src.a) return dest;
            if (src.a==255) return src;

            u8  a = src.a;
            
            RGBA col;
            col.a = a;

            col.r = ( (src.r*a) + (dest.r*(255-a)) ) >>8;
            col.g = ( (src.g*a) + (dest.g*(255-a)) ) >>8;
            col.b = ( (src.b*a) + (dest.b*(255-a)) ) >>8;

            return col;
#else
            // not portable at all, but pretty fast
            // Bleh.  For some reason, it swaps red and blue.  No idea why.
            // I can no longer figure out how the hell this thing ever worked in the first place. ;P
            // I should try and fix this some time.
            u32 s, d, c;
            u8 a = src.a;
            __asm
            {
                    mov        ebx, [src]
                    mov        s, ebx           // s=*pSrc
                    mov        eax, [dest]
                    mov        d, eax           // d=*pDest
                    
                    // Mix the blue channel
                    mov        ecx, eax         // ebx = s  ecx = d        
                    and        ebx, 255         // ebx = s&255
                    and        ecx, 255         // ecx = d&255
                    
                    xor        ah, ah
                    mov        al, a
                    sub        bx, cx           // bx=(s&255-d&255)
                    mul        bx               // eax = bx*ax (where bx is set above, and ax is equal to the alpha)

                    shr        ax, 8            // ax= a*(s&255-d&255)/256
                    add        ax, cx           // al= a*(s&255-d&255)/256+d&255
                    and        eax, 255
                    
                    shl        eax, 16          // temp
                    
                    mov        c, eax           // save the blue chan
                    
                    // Green
                    mov        ebx, s           // ebx = s
                    mov        ecx, d           // ecx = d
                    shr        ebx, 8
                    shr        ecx, 8
                    mov        s, ebx           // s>>=8;
                    mov        d, ecx           // d>>=8;
                    
                    and        ebx, 255         // ebx = s&255
                    and        ecx, 255         // ecx = d&255
                    
                    xor        ax, ax
                    mov        al, a
                    sub        bx, cx           // bl=(s&255-d&255)
                    imul       bx               // ax = bl*al (where bl is set above, and al is equal to the alpha)
                    
                    shl        cx, 8
                    add        ax, cx           // ax= a*(s&255-d&255)+(d&255*256)
                    and        eax, 0xFF00
                    
                    or         c, eax           // mix in the green chan
                    
                    // Red
                    mov        ebx, s           // ebx = s
                    mov        ecx, d           // ecx = d
                    shr        ebx, 8
                    shr        ecx, 8
                    
                    and        ebx, 255         // ebx = s&255
                    and        ecx, 255         // ecx = d&255

                    xor        ax, ax
                    mov        al, a
                    sub        bx, cx           // bl=(s&255-d&255)
                    imul       bx               // ax = bl*al (where bl is set above, and al is equal to the alpha)
                    
                    shl        cx, 8
                    add        ax, cx           // ax= a*(s&255-d&255)+(d&255*256)
                    and        eax, 0xFF00
                    shl        eax, 8
                    
                    or         c, eax           // mix in the red chan

                    shr        eax, 16
                   
                    mov        eax, c
            }
            return RGBA(c);
#endif
        }
    };

    /*!
        Class designed to do all image rendering.
        The template argument is the blending method you want to use.
    */
    template <typename Blender>
    class CanvasBlitter
    {
        static inline void DoClipping(int& x, int& y, int& xstart, int& xlen, int& ystart, int& ylen, const Rect& rClip)
        {
            if (x<rClip.left)
            {
                xlen-=(rClip.left-x);
                xstart+=(rClip.left-x);
                x = rClip.left;
            }
            if (y<rClip.top)
            {
                ylen-=(rClip.top-y);
                ystart+=(rClip.top-y);
                y = rClip.top;
            }
            if (x+xlen>rClip.right)
                xlen = rClip.right-x;
            if (y+ylen>rClip.bottom)
                ylen = rClip.bottom-y;
        }

        static inline void keepinrange(int& i, int min, int max)
        {
            if (i<min) i = min;
            if (i>max) i = max;
        }

    public:

        //! Renders an image on another image.
        static void Blit(Canvas& src, Canvas& dest, int x, int y)
        {
            const Rect& r = src.getClipRect();

            int xstart = r.left;
            int ystart = r.top;
            int xlen = r.Width();
            int ylen = r.Height();

            DoClipping(x, y, xstart, xlen, ystart, ylen, dest.getClipRect());
            if (xlen < 1 || ylen < 1)    return;                         // offscreen

            RGBA* pSrc  = src.getPixels() + (ystart * src.getWidth()) + xstart;
            RGBA* pDest = dest.getPixels() + (y * dest.getWidth()) + x;
            int  srcinc = src.getWidth() - xlen;
            int  destinc = dest.getWidth() - xlen;

            while (ylen)
            {
                int x = xlen;
                while (x--)
                {
                    *pDest = Blender::Blend(*pSrc, *pDest);
                    pDest++;
                    pSrc++;
                }

                pSrc += srcinc;
                pDest += destinc;
                --ylen;
            }
        }

        //! Renders an image on another image, stretching as necessary.
        static void ScaleBlit(Canvas& src, Canvas& dest, int cx, int cy, int w, int h)
        {
            int x, y;          // current pixel position
            int ix, iy;        // current image location (fixed point 16.16)
            int xinc, yinc;    // Increment to ix, iy per screen pixel (fixed point)
            
            int xstart, ystart;    // position of first pixel to copy (on the image)
            int xlen, ylen;        // number of screen pixels to copy along each axis

            if (w==0 || h==0)   return;
            
            ix = iy = 0;
            x = cx; y = cy;

            const Rect& srcclip = src.getClipRect();
            
            xinc=((srcclip.Width())<<16)/w;
            yinc=((srcclip.Height())<<16)/h;

            xstart = srcclip.left;
            ystart = srcclip.top;
            xlen = w;
            ylen = h;

            DoClipping(x, y, xstart, xlen, ystart, ylen, dest.getClipRect());
            if (xlen<1 || ylen<1)    return;    // image is entirely offscreen

            int xs = xinc*xstart;
            int ys = yinc*ystart;
            
            ix = xs;
            iy = ys & 0xFFFF;
            
            RGBA* pSrc  =src.getPixels() + ((ys >> 16) * src.getWidth());
            RGBA* pDest =dest.getPixels() + (y * dest.getWidth()) + x;
            
            x = 0;
            y = h;
            
            while (ylen)
            {
                while (x<xlen)
                {
                    pDest[x]=Blender::Blend(pDest[x], pSrc[ix>>16]);
                    ix+=xinc;
                    ++x;
                }
                
                x = 0;
                ix = xs;
                
                pDest+=dest.getWidth();
                
                iy+=yinc;
                pSrc+=(iy>>16)*src.getWidth();
                iy&=0xFFFF;
                --ylen;
            }
        }

        // ------------------------------------ Primatives ------------------------------------

        //! Draws a dot on the image.
        static inline void SetPixel(Canvas& img, int x, int y, RGBA colour)
        {
            RGBA* p = img.getPixels()+(y*img.getWidth() + x);

            *p = Blender::Blend(colour, *p);
        }

        //! Draws a horizontal line on the image.
        static inline void HLine(Canvas& img, int x1, int x2, int y, RGBA colour)
        {
            const Rect& r = img.getClipRect();

            if (y<r.top || y>=r.bottom)
                return;

            if (x1>x2)
                std::swap(x1, x2);

            keepinrange(x1, r.left, r.right-1);
            keepinrange(x2, r.left, r.right-1);

            RGBA* p = img.getPixels()+(y*img.getWidth())+x1;

            int xlen = x2-x1;

            while (xlen--)
            {
                *p = Blender::Blend(colour, *p);
                p++;
            }

        }

        //! Draws a vertical line on the image.
        static inline void VLine(Canvas& img, int x, int y1, int y2, RGBA colour)
        {
            const Rect& r = img.getClipRect();

            if (x<r.left || x>=r.right) return;

            if (y1>y2)
                std::swap(y1, y2);

            keepinrange(y1, r.top, r.bottom-1);
            keepinrange(y2, r.top, r.bottom-1);

            RGBA* p = img.getPixels()+(y1*img.getWidth())+x;

            int yinc = img.getWidth();

            int ylen = y2-y1;

            while (ylen--)
            {
                *p = Blender::Blend(colour, *p);
                p+=yinc;
            }
        }

        //! Draws a rectangle (outline or filled) on the image.
        static void DrawRect(Canvas& img, int x1, int y1, int x2, int y2, RGBA colour, bool filled)
        {
            if (filled)
            {
                int ydir = y1<y2?1:-1;
                for (int y = y1; y!=y2; y+=ydir)
                    HLine(img, x1, x2, y, colour);
            }
            else
            {
                if (abs(y1-y2)>1)
                {
                    VLine(img, x1, y1+1, y2-1, colour);
                    VLine(img, x2, y1+1, y2-1, colour);
                }
                else
                {
                    HLine(img, x1, x2, y1, colour);
                    return;
                }

                if (abs(x1-x2)>1)
                {
                    HLine(img, x1, x2, y1, colour);
                    HLine(img, x1, x2, y2, colour);
                }
                else
                {
                    VLine(img, x1, y1, y2, colour);
                    return;
                }
            }
        }

        /*!
            Draws an arbitrary line on the image.
            Kudos to zeromus for the algorithm.
        */
        static void Line(Canvas& img, int x1, int y1, int x2, int y2, u32 colour)
        {
            // check for the cases in which the line is vertical or horizontal or only one pixel big
            // we can do those faster through other means
            if(y1==y2 && x1==x2)
            {
                SetPixel(img, x1, y1, colour);
                return;
            }
            if(y1==y2)
            {
                HLine(img, x1, x2, y1, colour);
                return;
            }
            if(x1==x2)
            {
                VLine(img, x1, y1, y2, colour);
                return;
            }

            const Rect& r = img.getClipRect();
            
            // it's good to have these handy
            int cx1=r.left;
            int cx2=r.right-1;
            int cy1=r.top;
            int cy2=r.bottom-1;
            
            // variables for the clipping code
            int v1=0, v2=0;
            
            // here begins what is apparently the Cohen-Sutherland line clipping algorithm
            // there's a doc on it here: http://reality.sgi.com/tomcat_asd/algorithms.html#clipping
            if(y1<cy1) v1|=8;
            if(y1>cy2) v1|=4;
            if(x1>cx2) v1|=2;
            if(x1<cx1) v1|=1;
            if(y2<cy1) v2|=8;
            if(y2>cy2) v2|=4;
            if(x2>cx2) v2|=2;
            if(x2<cx1) v2|=1;
            
            while((v1&v2)==0&&(v1|v2)!=0)
            {
                if(v1)
                {
                    if(v1&8)      // clip above
                    {
                        x1-=((x1-x2)*(cy1-y1))/(y2-y1+1);
                        y1=cy1;
                    }
                    else if(v1&4) // clip below
                    {
                        x1-=((x1-x2)*(y1-cy2))/(y1-y2+1);
                        y1=cy2;
                    }
                    else if(v1&2) // clip right
                    {
                        y1-=((y1-y2)*(x1-cx2))/(x1-x2+1);
                        x1=cx2;
                    }
                    else          // clip left
                    {
                        y1-=((y1-y2)*(cx1-x1))/(x2-x1+1);
                        x1=cx1;
                    }
                    v1=0;
                    if(y1<cy1) v1|=8;
                    if(y1>cy2) v1|=4;
                    if(x1>cx2) v1|=2;
                    if(x1<cx1) v1|=1;
                }
                else
                {
                    if(v2&8)      // clip above
                    {
                        x2-=((x2-x1)*(cy1-y2))/(y1-y2+1);
                        y2=cy1;
                    }
                    else if(v2&4) // clip below
                    {
                        x2-=((x2-x1)*(y2-cy2))/(y2-y1+1);
                        y2=cy2;
                    }
                    else if(v2&2) // clip right
                    {
                        y2-=((y2-y1)*(x2-cx2))/(x2-x1+1);
                        x2=cx2;
                    }
                    else          // clip left
                    {
                        y2-=((y2-y1)*(cx1-x2))/(x1-x2+1);
                        x2=cx1;
                    }
                    v2=0;
                    if(y2<cy1) v2|=8;
                    if(y2>cy2) v2|=4;
                    if(x2>cx2) v2|=2;
                    if(x2<cx1) v2|=1;
                }
            }
            
            // this tells us if the line was clipped in its entirety. yum!
            if(v1&v2)
                return;
            
            //make these checks again
            if(y1==y2&&x1==x2)
            {
                SetPixel(img, x1, y1, colour);
                return;
            }
            if(y1==y2)
            {
                HLine(img, x1, x2, y1, colour);
                return;
            }
            if(x1==x2)
            {
                VLine(img, x1, y1, y2, colour);
                return;
            }
            
            //for the line renderer
            int xi, yi, xyi;
            int d, dir, diu, dx, dy;
            
            //two pointers because we render the line from both ends
            /*RGBA* w1=&(img.getPixels())[img.getWidth()*y1+x1];
            RGBA* w2=&(img.getPixels())[img.getWidth()*y2+x2];*/
            RGBA* w1=img.getPixels() + (img.getWidth()*y1+x1);
            RGBA* w2=img.getPixels() + (img.getWidth()*y2+x2);

            //start algorithm presently.
            xi = 1;
            if((dx = x2-x1)<0)
            {
                dx=-dx;
                xi=-1;
            }
            
            yi = img.getWidth();
            if((dy = y2-y1)<0)
            {
                yi=-yi;
                dy=-dy;
            }
            
            xyi = xi+yi;
            
            if(dy<dx)
            {
                dir = dy*2;
                d=-dx;
                diu = 2*d;
                dy = dx/2;
                
                for(;;)
                {
                    *w1=colour;
                    *w2=colour;
                    if((d+=dir)<=0)
                    {
                        w1+=xi;
                        w2-=xi;
                        if((--dy)>0) continue;
                        *w1=colour;
                        if((dx&1)==0) return;
                        *w2=colour;
                        return;
                    }
                    else
                    {
                        w1+=xyi;
                        w2-=xyi;
                        d+=diu;
                        if((--dy)>0) continue;
                        *w1=colour;
                        if((dx&1)==0) return;
                        *w2=colour;
                        return;
                    }
                }
            }
            else
            {
                dir = dx*2;
                d=-dy;
                diu = d*2;
                dx = dy/2;
                for(;;)
                {
                    *w1=colour;
                    *w2=colour;
                    if((d+=dir)<=0)
                    {
                        w1+=yi;
                        w2-=yi;
                        if((--dx)>0) continue;
                        *w1=colour;
                        if((dy&1)==0) return;
                        *w2=colour;
                        return;
                    }
                    else
                    {
                        w1+=xyi;
                        w2-=xyi;
                        d+=diu;
                        if((--dx)>0) continue;
                        *w1=colour;
                        if((dy&1)==0) return;
                        *w2=colour;
                        return;
                    }
                }
            }
            return;
        }
    };

}


#endif
