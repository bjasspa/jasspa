// gcc `pkg-config --cflags gtk+-3.0` -o font-styles font-styles.c `pkg-config --libs gtk+-3.0`            
//////////////////////////////////////////////////////////////////////////////
#include <gtk/gtk.h>
#include <pango/pango.h>
#include <string.h>
#include <stdio.h>
int main(int argc, char *argv[]) { 
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    PangoFontFamily **families;
    int n_families = 0;
    pango_font_map_list_families(fontmap, &families, &n_families);

    for (int i = 0; i < n_families; ++i) {
        if (!pango_font_family_is_monospace(families[i])) continue;
        // Only include if the name contains the target keywords
        const char *family_name = pango_font_family_get_name(families[i]);
        if (!(strstr(family_name, "Proggy") ||
              strstr(family_name, "Courier") ||
              strstr(family_name, "Sudo") ||              
              strstr(family_name, "Inconsol") ||                            
              strstr(family_name, "Terminus") ||              
              strstr(family_name, "Code") ||
              strstr(family_name, "Anonymous") ||              
              strstr(family_name, "Typewriter") ||              
              strstr(family_name, "Mon") ||                            
              strstr(family_name, "Consol"))) {
              continue;
        }
        //printf("family: %s\n",family_name);
        PangoFontFace **faces;
        int n_faces = 0;
        pango_font_family_list_faces(families[i], &faces, &n_faces);
        for (int i = 0; i < n_faces; ++i) {
            printf("%s %s\n",family_name, pango_font_face_get_face_name(faces[i]));
        }
    }
    return 0;
}
