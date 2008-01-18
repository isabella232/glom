/* This is copied from GTK+ 2.15/16 so we can use new API without waiting 
 * for stable release of GTK+ 2.16. Life would be easier if GTK+ followed 
 * the GNOME release schedule.
 */
#include <gtk/gtkcalendar.h>
#if GTK_CHECK_VERSION(2,16,0)
typedef GlomGlomGtkCalendar GlomGlomGtkCalendar; 
#else
/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * GTK Calendar Widget
 * Copyright (C) 1998 Cesar Miquel and Shawn T. Amundson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __GLOM_GTK_CALENDAR_H__
#define __GLOM_GTK_CALENDAR_H__

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>

/* Not needed, retained for compatibility -Yosh */
#include <gtk/gtksignal.h>


G_BEGIN_DECLS

#define GLOM_GTK_TYPE_CALENDAR                  (glom_gtk_calendar_get_type ())
#define GLOM_GTK_CALENDAR(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GLOM_GTK_TYPE_CALENDAR, GlomGtkCalendar))
#define GLOM_GTK_CALENDAR_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GLOM_GTK_TYPE_CALENDAR, GlomGtkCalendarClass))
#define GLOM_GTK_IS_CALENDAR(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GLOM_GTK_TYPE_CALENDAR))
#define GLOM_GTK_IS_CALENDAR_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GLOM_GTK_TYPE_CALENDAR))
#define GLOM_GTK_CALENDAR_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GLOM_GTK_TYPE_CALENDAR, GlomGtkCalendarClass))


typedef struct _GlomGtkCalendar	       GlomGtkCalendar;
typedef struct _GlomGtkCalendarClass       GlomGtkCalendarClass;

typedef struct _GlomGtkCalendarPrivate     GlomGtkCalendarPrivate;

/**
 * GlomGtkCalendarDisplayOptions:
 * @GLOM_GTK_CALENDAR_SHOW_HEADING: Specifies that the month and year should be displayed.
 * @GLOM_GTK_CALENDAR_SHOW_DAY_NAMES: Specifies that three letter day descriptions should be present.
 * @GLOM_GTK_CALENDAR_NO_MONTH_CHANGE: Prevents the user from switching months with the calendar.
 * @GLOM_GTK_CALENDAR_SHOW_WEEK_NUMBERS: Displays each week numbers of the current year, down the
 * left side of the calendar.
 * @GLOM_GTK_CALENDAR_WEEK_START_MONDAY: Since GTK+ 2.4, this option is deprecated and ignored by GTK+.
 * The information on which day the calendar week starts is derived from the locale.
 * @GLOM_GTK_CALENDAR_SHOW_DETAILS: Just show an indicator, not the full details
 * text when details are provided. See glom_gtk_calendar_set_detail_func().
 *
 * These options can be used to influence the display and behaviour of a #GlomGtkCalendar.
 */
typedef enum
{
  GLOM_GTK_CALENDAR_SHOW_HEADING		= 1 << 0,
  GLOM_GTK_CALENDAR_SHOW_DAY_NAMES		= 1 << 1,
  GLOM_GTK_CALENDAR_NO_MONTH_CHANGE		= 1 << 2,
  GLOM_GTK_CALENDAR_SHOW_WEEK_NUMBERS	= 1 << 3,
  GLOM_GTK_CALENDAR_WEEK_START_MONDAY	= 1 << 4,
  GLOM_GTK_CALENDAR_SHOW_DETAILS		= 1 << 5,
} GlomGtkCalendarDisplayOptions;

/**
 * GlomGtkCalendarDetailFunc:
 * @calendar: a #GlomGtkCalendar.
 * @year: the year for which details are needed.
 * @month: the month for which details are needed.
 * @day: the day of @month for which details are needed.
 * @user_data: the data passed with glom_gtk_calendar_set_detail_func().
 *
 * This kind of functions provide Pango markup with detail information for the
 * specified day. Examples for such details are holidays or appointments. The
 * function returns %NULL when no information is available.
 *
 * Since: 2.16
 *
 * Return value: Newly allocated string with Pango markup with details
 * for the specified day, or %NULL.
 */
typedef gchar* (*GlomGtkCalendarDetailFunc) (GlomGtkCalendar *calendar,
                                         guint        year,
                                         guint        month,
                                         guint        day,
                                         gpointer     user_data);

struct _GlomGtkCalendar
{
  GtkWidget widget;
  
  GtkStyle  *header_style;
  GtkStyle  *label_style;
  
  gint month;
  gint year;
  gint selected_day;
  
  gint day_month[6][7];
  gint day[6][7];
  
  gint num_marked_dates;
  gint marked_date[31];
  GlomGtkCalendarDisplayOptions  display_flags;
  GdkColor marked_date_color[31];
  
  GdkGC *gc;			/* unused */
  GdkGC *xor_gc;		/* unused */

  gint focus_row;
  gint focus_col;

  gint highlight_row;
  gint highlight_col;
  
  GlomGtkCalendarPrivate *priv;
  gchar grow_space [32];

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

struct _GlomGtkCalendarClass
{
  GtkWidgetClass parent_class;
  
  /* Signal handlers */
  void (* month_changed)		(GlomGtkCalendar *calendar);
  void (* day_selected)			(GlomGtkCalendar *calendar);
  void (* day_selected_double_click)	(GlomGtkCalendar *calendar);
  void (* prev_month)			(GlomGtkCalendar *calendar);
  void (* next_month)			(GlomGtkCalendar *calendar);
  void (* prev_year)			(GlomGtkCalendar *calendar);
  void (* next_year)			(GlomGtkCalendar *calendar);
  
};


GType	   glom_gtk_calendar_get_type	(void) G_GNUC_CONST;
GtkWidget* glom_gtk_calendar_new		(void);

gboolean   glom_gtk_calendar_select_month	(GlomGtkCalendar *calendar, 
					 guint	      month,
					 guint	      year);
void	   glom_gtk_calendar_select_day	(GlomGtkCalendar *calendar,
					 guint	      day);

gboolean   glom_gtk_calendar_mark_day	(GlomGtkCalendar *calendar,
					 guint	      day);
gboolean   glom_gtk_calendar_unmark_day	(GlomGtkCalendar *calendar,
					 guint	      day);
void	   glom_gtk_calendar_clear_marks	(GlomGtkCalendar *calendar);


void	   glom_gtk_calendar_set_display_options (GlomGtkCalendar    	      *calendar,
					     GlomGtkCalendarDisplayOptions flags);
GlomGtkCalendarDisplayOptions
           glom_gtk_calendar_get_display_options (GlomGtkCalendar   	      *calendar);
#ifndef GTK_DISABLE_DEPRECATED
void	   glom_gtk_calendar_display_options (GlomGtkCalendar		  *calendar,
					 GlomGtkCalendarDisplayOptions flags);
#endif

void	   glom_gtk_calendar_get_date	(GlomGtkCalendar *calendar, 
					 guint	     *year,
					 guint	     *month,
					 guint	     *day);

void       glom_gtk_calendar_set_detail_func (GlomGtkCalendar           *calendar,
                                         GlomGtkCalendarDetailFunc  func,
                                         gpointer               data,
                                         GDestroyNotify         destroy);

void       glom_gtk_calendar_set_detail_width_chars (GlomGtkCalendar    *calendar,
                                                gint            chars);
void       glom_gtk_calendar_set_detail_height_rows (GlomGtkCalendar    *calendar,
                                                gint            rows);

gint       glom_gtk_calendar_get_detail_width_chars (GlomGtkCalendar    *calendar);
gint       glom_gtk_calendar_get_detail_height_rows (GlomGtkCalendar    *calendar);

#ifndef GTK_DISABLE_DEPRECATED
void	   glom_gtk_calendar_freeze		(GlomGtkCalendar *calendar);
void	   glom_gtk_calendar_thaw		(GlomGtkCalendar *calendar);
#endif

G_END_DECLS

#endif /* __GLOM_GTK_CALENDAR_H__ */

#endif //!GTK_CHECK_VERSION(2.16.0)
