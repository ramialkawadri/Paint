/* paint-application.c
 *
 * Copyright 2023 Rami Alkawadri
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "paint-application.h"
#include "paint-window.h"

struct _PaintApplication
{
  AdwApplication parent_instance;
};

G_DEFINE_TYPE (PaintApplication, paint_application, ADW_TYPE_APPLICATION)

PaintApplication *
paint_application_new (const char       *application_id,
                       GApplicationFlags flags)
{
  g_return_val_if_fail (application_id != NULL, NULL);

  return g_object_new (PAINT_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

static void
paint_application_activate (GApplication *app)
{
  GtkWindow *window;

  g_assert (PAINT_IS_APPLICATION(app));

  window = gtk_application_get_active_window (GTK_APPLICATION(app));

  if (window == NULL)
    { 
      window = g_object_new (PAINT_TYPE_WINDOW,
                             "application", app,
                             NULL);
    }

  gtk_window_present (window);
}

static void
paint_application_class_init (PaintApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->activate = paint_application_activate;
}

static void
paint_application_about_action (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       user_data)
{
  static const char *developers[] = { "Rami Alkawadri", NULL };
  PaintApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (PAINT_IS_APPLICATION(self));

  window = gtk_application_get_active_window (GTK_APPLICATION(self));

  adw_show_about_window (window,
                         "application-name", "Paint",
                         "application-icon", "org.gnome.paint",
                         "developer-name", "Rami Alkawadri",
                         "version", "0.1.0",
                         "developers", developers,
                         "license-type", GTK_LICENSE_GPL_3_0,
                         "issue-url", "https://github.com/ramialkawadri/Paint/issues/new",
                         NULL);
}

static void
paint_application_exit_action (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
  PaintApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (PAINT_IS_APPLICATION(self));
  window = gtk_application_get_active_window (GTK_APPLICATION(self));
  gtk_window_close (window);

}

static void
paint_application_save_action (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
  PaintApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (PAINT_IS_APPLICATION(self));

  window = gtk_application_get_active_window (GTK_APPLICATION(self));
  paint_window_save_current_file (PAINT_WINDOW (window));
}

static void
paint_application_open_action (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
  PaintApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (PAINT_IS_APPLICATION(self));

  window = gtk_application_get_active_window (GTK_APPLICATION(self));

  paint_window_open_new_file (PAINT_WINDOW (window));
}

static void
paint_application_undo_action (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
  PaintApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (PAINT_IS_APPLICATION(self));

  window = gtk_application_get_active_window (GTK_APPLICATION(self));

  paint_window_undo (PAINT_WINDOW (window));
}

static void
paint_application_redo_action (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
  PaintApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (PAINT_IS_APPLICATION(self));

  window = gtk_application_get_active_window (GTK_APPLICATION(self));

  paint_window_redo (PAINT_WINDOW (window));
}

static const GActionEntry app_actions[] = {
    {"exit", paint_application_exit_action},
    {"about", paint_application_about_action},
    {"save", paint_application_save_action},
    {"open", paint_application_open_action},
    {"undo", paint_application_undo_action},
    {"redo", paint_application_redo_action},
};

static void
paint_application_init (PaintApplication *self)
{
  g_action_map_add_action_entries (G_ACTION_MAP(self),
                                   app_actions,
                                   G_N_ELEMENTS (app_actions),
                                   self);

  gtk_application_set_accels_for_action (GTK_APPLICATION(self),
                                         "app.save",
                                         (const char *[]){"<primary>s", NULL});

  gtk_application_set_accels_for_action (GTK_APPLICATION(self),
                                         "app.open",
                                         (const char *[]){"<primary>o", NULL});

  gtk_application_set_accels_for_action (GTK_APPLICATION(self),
                                         "app.undo",
                                         (const char *[]){"<primary>z", NULL});

  gtk_application_set_accels_for_action (GTK_APPLICATION(self),
                                         "app.redo",
                                         (const char *[]){"<primary>r", NULL});

  gtk_application_set_accels_for_action (GTK_APPLICATION(self),
                                         "app.exit",
                                         (const char *[]){"<primary>w", NULL});
}
