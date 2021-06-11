/*
 * 2002 Peyronnet Rémi 
 *
 * This plugin transforms your image RGB in YUV
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Feb 2002 : 1.0 : Initial release
 * Mar 2005 : 1.1 : Ported to GIMP 2.2
 * Jan 2010 : 1.3 : Patch from Martin Ramshaw (doc)
 *
 */

/* Many parts of this code are borrowed from other plugins source code. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "libgimp/gimp.h"

#define GET_MAX(x,y) ( ((x)>(y))?(x):(y) )
#define GET_MIN(x,y) ( ((x)<(y))?(x):(y) )


/** defines ***********************************************************/

#define PLUG_IN_NAME "plug_in_yuv"
#define PLUG_IN_VERSION "Jan. 2010, 1.3"


/** Plugin interface *********************************************************/

void query(void);
void run(const gchar *name, int nparams, const GimpParam *param, int *nreturn_vals, GimpParam **return_vals);


GimpPlugInInfo PLUG_IN_INFO = {
  NULL, /* init_proc */
  NULL, /* quit_proc */
  query,        /* query_proc */
  run   /* run_proc */
};


MAIN()

void
query(void)
{
  /* Definition of parameters */
  static GimpParamDef args[] = {
    { GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
    { GIMP_PDB_IMAGE, "image", "Input image (unused)" },
    { GIMP_PDB_DRAWABLE, "drawable", "Input drawable" }
  };

  static GimpParamDef *return_vals  = NULL;
  static int        nargs = sizeof(args) / sizeof(args[0]);
  static int        nreturn_vals = 0;

  gimp_install_procedure(
    "plug_in_rgb_yuv",
    "Transform the image from RGB to YUV.\n\nAfter having applied the filter, you will get luminance (Y) in the red channel (R), and chrominances in the green and blue channels.\n\nBy using the decompose plugin, or the channel dialog, you will be able to work in YUV space, and then convert back to RGB space with the reverse plugin.",
    "This plugin replaces the RGB channels with YUV values.",
    "Remi Peyronnet",
    "Remi Peyronnet",
    PLUG_IN_VERSION,
    "<Image>/Image/Mode/RGB->YUV",
    "RGB*",
    GIMP_PLUGIN,
    nargs,
    nreturn_vals,
    args,
    return_vals);
  gimp_install_procedure(
    "plug_in_yuv_rgb",
    "Transform the image from YUV to RGB.\n\nEffectively, this re-maps the image back into the normal RGB channels from YUV.",
    "This plugin replaces the RGB channels of an YUV image with the good RGB values.",
    "Remi Peyronnet",
    "Remi Peyronnet",
    PLUG_IN_VERSION,
    "<Image>/Image/Mode/YUV->RGB",
    "RGB*",
    GIMP_PLUGIN,
    nargs,
    nreturn_vals,
    args,
    return_vals);
}

void
run(const gchar *name, int nparams, const GimpParam *param,
    int *nreturn_vals, GimpParam **return_vals)
{
  /* Return values */
  static GimpParam values[1];

  gint sel_x1, sel_y1, sel_x2, sel_y2;
  gint img_height, img_width, img_bpp, img_has_alpha;

  GimpDrawable     *drawable;
  GimpPixelRgn dest_rgn, src_rgn, *pr; 
  GimpRunMode  run_mode;
  GimpPDBStatusType   status;

  double progress, max_progress;

  guchar * dest_row, *src_row, *dest, *src;
  double  r,g,b,a=0,y,u,v; 
  gint row, col;
  
  *nreturn_vals = 1;
  *return_vals  = values;

  status = GIMP_PDB_SUCCESS;
        
  if (param[0].type!= GIMP_PDB_INT32)  status=GIMP_PDB_CALLING_ERROR;
  if (param[2].type!=GIMP_PDB_DRAWABLE)   status=GIMP_PDB_CALLING_ERROR;

  run_mode = (GimpRunMode) param[0].data.d_int32;
  
  drawable = gimp_drawable_get(param[2].data.d_drawable);

  img_width     = gimp_drawable_width(drawable->drawable_id);
  img_height    = gimp_drawable_height(drawable->drawable_id);
  img_bpp       = gimp_drawable_bpp(drawable->drawable_id);
  img_has_alpha = gimp_drawable_has_alpha(drawable->drawable_id);
  gimp_drawable_mask_bounds(drawable->drawable_id, &sel_x1, &sel_y1, &sel_x2, &sel_y2);

  max_progress = (sel_x2-sel_x1)*(sel_y2-sel_y1);

  
  if (status == GIMP_PDB_SUCCESS)
  {
    // Tile 
    gimp_tile_cache_ntiles((drawable->width + gimp_tile_width() - 1) / gimp_tile_width());


    if (strcmp("plug_in_rgb_yuv",name) == 0)
    {
      // RGB -> YUV
      // !!! Warning !!! Duplicated code... 'cause it'is quick'n dirty :)
        gimp_progress_init("Converting RGB to YUV...");
        progress = 0;
        
        // Process
        gimp_pixel_rgn_init (&dest_rgn, drawable, sel_x1, sel_y1, (sel_x2-sel_x1), (sel_y2-sel_y1), TRUE, TRUE);
        gimp_pixel_rgn_init (&src_rgn, drawable, sel_x1, sel_y1, (sel_x2-sel_x1), (sel_y2-sel_y1), FALSE, FALSE);
        
        // Methode de traitement par dest_rgns -----------------------
        for (pr = (GimpPixelRgn *) gimp_pixel_rgns_register (2, &src_rgn, &dest_rgn);
             pr != NULL;
             pr = (GimpPixelRgn *) gimp_pixel_rgns_process (pr)) 
         { //Fun Goes On Here
           dest_row = dest_rgn.data;
           src_row = src_rgn.data;
           for (row = 0; row < dest_rgn.h; row++) {
             dest = dest_row;
             src = src_row;
             for (col = 0; col < dest_rgn.w; col++) {

               // Début du traitement spécifique *************
               r = *src++; //y 
               g = *src++; //u
               b = *src++; //v
               if (img_has_alpha)	a = *src++;

               /* First set of formula, probably not the best... ----
               y =   (0.257*r) + (0.504*g) + (0.098*b) + 16;
               u =   (0.439*r) - (0.368*g) + (0.071*b) + 128;
               v = - (0.148*r) - (0.291*g) + (0.439*b) + 128;

               // YUV->RGB
               // r = 1.164 * (y-16) + 1.596*(v-128);
               // g = 1.164 * (y-16) + 0.813*(v-128) - 0.391*(u-128);
               // b = 1.164 * (y-16) + 2.018*(u-128);
               */

               /* Second set, not much better...*/
               y =   (0.299*r) + (0.587*g) + (0.114*b);
               u =  -(0.169*r) - (0.331*g) + (0.500*b) + 128.0;
               v =   (0.500*r) - (0.419*g) - (0.081*b) + 128.0;
               
               // YUV->RGB
               //r = y + 1.402*(v-128.0);
               //g = y - 0.34414*(u-128.0) + 0.71414*(v-128.0);
               //b = y + 1.772*(u-128.0);
               //
			   // From SciLab : This is the good one.
			   //r = 1 * y -  0.0009267*(u-128)  + 1.4016868*(v-128);
			   //g = 1 * y -  0.3436954*(u-128)  - 0.7141690*(v-128);
			   //b = 1 * y +  1.7721604*(u-128)  + 0.0009902*(v-128);

               /** Third : home-made...*/
			   /*y = 0.333 * r + 0.333 * g + 0.333 * b;
			   u = r - y;
			   v = g - y;
			   r = y + u;
			   g = y + v;
			   b = y -u -v;
			   */

               *dest++ = (guchar)((y>255)?255:((y<0)?0:y));
               *dest++ = (guchar)((u>255)?255:((u<0)?0:u));
               *dest++ = (guchar)((v>255)?255:((v<0)?0:v));
               
               if (img_has_alpha)	*dest++ = (guchar)a;
               // Fin du traitement spécifique ****************
               
           } // for
           dest_row += dest_rgn.rowstride;
           src_row += src_rgn.rowstride;
          } // for 
          // Update progress 
          progress += dest_rgn.w * dest_rgn.h;
          gimp_progress_update((double) progress / max_progress);
          //printf("%f / %f = %f %%\n",(float)progress,(float)max_progress,(float)(double) progress / max_progress);
       }
    
    }
    else if (strcmp("plug_in_yuv_rgb",name) == 0)
    {
      // RGB -> YUV
      // !!! Warning !!! Duplicated code... 'cause it'is quick'n dirty :)
      // You should consider just edit the previous version and copy/paste this one.
        gimp_progress_init("Converting YUV to RGB...");
        progress = 0;
        
        // Process
        gimp_pixel_rgn_init (&dest_rgn, drawable, sel_x1, sel_y1, (sel_x2-sel_x1), (sel_y2-sel_y1), TRUE, TRUE);
        gimp_pixel_rgn_init (&src_rgn, drawable, sel_x1, sel_y1, (sel_x2-sel_x1), (sel_y2-sel_y1), FALSE, FALSE);
        
        // Methode de traitement par dest_rgns -----------------------
        for (pr = (GimpPixelRgn *)gimp_pixel_rgns_register (2, &src_rgn, &dest_rgn);
             pr != NULL;
             pr = (GimpPixelRgn *)gimp_pixel_rgns_process (pr)) 
         { //Fun Goes On Here
           dest_row = dest_rgn.data;
           src_row = src_rgn.data;
           for (row = 0; row < dest_rgn.h; row++) {
             dest = dest_row;
             src = src_row;
             for (col = 0; col < dest_rgn.w; col++) {

               // Début du traitement spécifique *************
               y = *src++;
               u = *src++;
               v = *src++;
               if (img_has_alpha)	a = *src++;

               /* Second set, not much better...*/
               //y =   (0.299*r) + (0.587*g) + (0.114*b);
               //u =  -(0.169*r) - (0.331*g) + (0.500*b) + 128.0;
               //v =   (0.500*r) - (0.419*g) - (0.081*b) + 128.0;
               // From SciLab
               
				 r = 1 * y -  0.0009267*(u-128)  + 1.4016868*(v-128);
				 g = 1 * y -  0.3436954*(u-128)  - 0.7141690*(v-128);
				 b = 1 * y +  1.7721604*(u-128)  + 0.0009902*(v-128);

               
               *dest++ = (guchar)((r>255)?255:((r<0)?0:r));
               *dest++ = (guchar)((g>255)?255:((g<0)?0:g));
               *dest++ = (guchar)((b>255)?255:((b<0)?0:b));
               if (img_has_alpha)	*dest++ = (guchar)a;
               // Fin du traitement spécifique ****************
               
           } // for
           dest_row += dest_rgn.rowstride;
           src_row += src_rgn.rowstride;
          } // for 
          // Update progress 
          progress += dest_rgn.w * dest_rgn.h;
          gimp_progress_update((double) progress / max_progress);

       }
      
    }
    else
    {
       // Ouch, ugly :)
       printf("Plugin not found.\n");
    }

    gimp_drawable_flush(drawable);
    gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
    gimp_drawable_update (drawable->drawable_id, sel_x1, sel_y1, (sel_x2-sel_x1), (sel_y2-sel_y1));
    gimp_displays_flush();
  }
  
  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
  gimp_drawable_detach(drawable);
}
