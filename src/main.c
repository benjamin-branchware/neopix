#include "config.h"
#include <gtk/gtk.h>

#define CANVAS_WIDTH    32
#define CANVAS_HEIGHT   32
#define PIXEL_SIZE      20

static GdkRGBA pixels[CANVAS_WIDTH * CANVAS_HEIGHT];
static GdkRGBA current_color;

static void on_draw(GtkDrawingArea *area,
                    cairo_t        *cr,
                    int             width,
                    int             height,
                    gpointer        user_data) {
  for (int y = 0; y < CANVAS_HEIGHT; y++) {
    for (int x = 0; x < CANVAS_WIDTH; x++) {
      GdkRGBA *c = &pixels[y * CANVAS_WIDTH + x];
      gdk_cairo_set_source_rgba(cr, c);
      cairo_rectangle(cr, x * PIXEL_SIZE, y * PIXEL_SIZE,
                      PIXEL_SIZE, PIXEL_SIZE);
      cairo_fill(cr);

      cairo_rectangle(cr, x * PIXEL_SIZE, y * PIXEL_SIZE,
                      PIXEL_SIZE, PIXEL_SIZE);

      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_stroke(cr);
    }
  }
}

static void on_gesture_pressed(GtkGestureClick *gesture,
                               int npress,
                               double x, double y,
                               gpointer user_data) {
  GtkWidget *area = GTK_WIDGET(user_data);
  int xi = (int)(x / PIXEL_SIZE);
  int yi = (int)(y / PIXEL_SIZE);
  if (xi < 0 || xi >= CANVAS_WIDTH ||
      yi < 0 || yi >= CANVAS_HEIGHT)
    return;

  pixels[yi * CANVAS_WIDTH + xi] = current_color;
  gtk_widget_queue_draw(area);
}

static void on_color_set(GtkColorButton *btn,
                         gpointer user_data) {
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(btn), &current_color);
}

static void on_clear_clicked(GtkButton *btn,
                              gpointer user_data) {
  GtkWidget *area = GTK_WIDGET(user_data);
  GdkRGBA white = {1, 1, 1, 1};
  for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++)
    pixels[i] = white;
  gtk_widget_queue_draw(area);
}

static void handle_save_dialog(GObject *source_object, GAsyncResult *result, gpointer user_data) {
  GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);

  if (GFile *file = gtk_file_dialog_save_finish(dialog, result, NULL); file != NULL) {
    char *filename = g_file_get_path(file);
    g_object_unref(file);

    cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, CANVAS_WIDTH, CANVAS_HEIGHT);

    cairo_t *cr = cairo_create(surface);

    for (int y = 0; y < CANVAS_HEIGHT; y++) {
      for (int x = 0; x < CANVAS_WIDTH; x++) {
        GdkRGBA *c = &pixels[y * CANVAS_WIDTH + x];
        gdk_cairo_set_source_rgba(cr, c);
        cairo_rectangle(cr, x, y, 1, 1);
        cairo_fill(cr);
      }
    }

    cairo_destroy(cr);
    cairo_surface_write_to_png(surface, filename);
    cairo_surface_destroy(surface);
    g_free(filename);
  }
}

static void on_save_clicked(GtkButton *btn,
                            gpointer user_data) {
  GtkWindow *win = GTK_WINDOW(user_data);
  GtkFileDialog *dialog = gtk_file_dialog_new();

  gtk_file_dialog_set_accept_label(GTK_FILE_DIALOG(dialog), "_Save");
  gtk_file_dialog_set_title(GTK_FILE_DIALOG(dialog), "Save As");
  gtk_file_dialog_set_initial_name(GTK_FILE_DIALOG(dialog), "untitled.png");

  gtk_file_dialog_save(GTK_FILE_DIALOG(dialog), win, NULL,
                       (GAsyncReadyCallback) handle_save_dialog,
                       dialog);
}

static void on_activate(GtkApplication *app,
                        gpointer user_data) {
  GdkRGBA white = {1, 1, 1, 1};
  for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++)
    pixels[i] = white;
  current_color = (GdkRGBA){0, 0, 0, 1};

  GtkWidget *win = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(win), "Neopix Pixel Art Editor");
  gtk_window_set_default_size(GTK_WINDOW(win), CANVAS_WIDTH * PIXEL_SIZE, CANVAS_HEIGHT * PIXEL_SIZE + 50);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_window_set_child(GTK_WINDOW(win), vbox);

  /* Drawing area */
  GtkWidget *area = gtk_drawing_area_new();
  gtk_drawing_area_set_content_width(
      GTK_DRAWING_AREA(area),
      CANVAS_WIDTH * PIXEL_SIZE);
  gtk_drawing_area_set_content_height(
      GTK_DRAWING_AREA(area),
      CANVAS_HEIGHT * PIXEL_SIZE);

  gtk_drawing_area_set_draw_func(
                                 GTK_DRAWING_AREA(area),
                                 (GtkDrawingAreaDrawFunc)on_draw,
                                 NULL, NULL);

  GtkGestureClick *click = GTK_GESTURE_CLICK(gtk_gesture_click_new());
  gtk_gesture_single_set_button(
                                GTK_GESTURE_SINGLE(click),
                                GDK_BUTTON_PRIMARY
                                );
  gtk_widget_add_controller(
                            area,
                            GTK_EVENT_CONTROLLER(click)
                            );
  g_signal_connect(click, "pressed",
                   G_CALLBACK(on_gesture_pressed), area);

  /* Toolbar */
  GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_append(GTK_BOX(vbox), toolbar);

  /* Color-picker */
  GtkWidget *color_btn =
    gtk_color_button_new_with_rgba(&current_color);
  g_signal_connect(color_btn, "color-set", G_CALLBACK(on_color_set), NULL);
  gtk_box_append(GTK_BOX(toolbar), color_btn);

  /* Clear button */
  GtkWidget *clear = gtk_button_new_with_label("Clear");
  g_signal_connect(clear, "clicked", G_CALLBACK(on_clear_clicked), area);
  gtk_box_append(GTK_BOX(toolbar), clear);

  /* Save button */
  GtkWidget *save = gtk_button_new_with_label("Save");
  g_signal_connect(save, "clicked", G_CALLBACK(on_save_clicked), win);
  gtk_box_append(GTK_BOX(toolbar), save);

  gtk_box_append(GTK_BOX(vbox), area);

  gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char *argv[]) {
  GtkApplication *app =
    gtk_application_new("com.morsoft.neopix",
                        G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

  int status = g_application_run(
                                 G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
