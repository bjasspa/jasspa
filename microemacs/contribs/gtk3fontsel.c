// gcc `pkg-config --cflags gtk+-3.0` -o fontchooser_example gtk3fontsel.c `pkg-config --libs gtk+-3.0`
#include <gtk/gtk.h>
#include <pango/pango.h>
#include <stdio.h>
gboolean filter_monospace(const PangoFontFamily *family, const PangoFontFace *face, gpointer user_data) {
    return pango_font_family_is_monospace(family);
}

void on_font_set(GtkFontChooser *chooser, gpointer label) {
    gchar *fontname = gtk_font_chooser_get_font(chooser);
    gtk_label_set_text(GTK_LABEL(label), fontname);
    GtkWidget *widget = GTK_WIDGET(label);
    PangoFontDescription *desc = pango_font_description_from_string(fontname);
    gtk_widget_override_font(widget, desc);
    pango_font_description_free(desc);
    printf("%s\n",fontname);
    g_free(fontname);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Monospace Font Chooser");
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *label = gtk_label_new("Monospace font demo");

    GtkWidget *font_button = gtk_font_button_new();
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), font_button, FALSE, FALSE, 0);

    gtk_font_chooser_set_filter_func(GTK_FONT_CHOOSER(font_button),
        filter_monospace, NULL, NULL);
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(font_button), "Monospace 12");
    g_signal_connect(font_button, "font-set", G_CALLBACK(on_font_set), label);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
