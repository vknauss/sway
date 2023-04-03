#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include "sway/commands.h"
#include "sway/config.h"
#include "log.h"
#include "stringop.h"
#include <pango/pangocairo.h>

struct cmd_results *output_cmd_font(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "font", EXPECTED_AT_LEAST, 1))) {
		return error;
	}
    struct output_config *oc = config->handler_context.output_config;
    if (!oc)
    {
		return cmd_results_new(CMD_FAILURE, "Missing output config");
	}
    
	if (!sway_assert(argc > 0, "argc should be positive")) {
		return NULL;
	}
	int len = 0, i;
    int last_match_pos = 0;
	for (i = 0; i < argc; ++i) {
        char* temp = argv[i];
        char const *sep = ";";
        char match = 0;
        argsep(&temp, sep, &match);
        if (match)
        {
            last_match_pos = temp - argv[i];
            len += last_match_pos;
            break;
        }
		len += strlen(argv[i]) + 1;
	}
	char *font = malloc(len);
	len = 0;
    int count = i < argc ? i + 1 : argc;
	for (i = 0; i < count; ++i) {
        if (i + 1 == count && last_match_pos)
        {
            strncpy(font + len, argv[i], last_match_pos - 1);
            len += last_match_pos - 1;
        }
        else
        {
            strcpy(font + len, argv[i]);
            len += strlen(argv[i]);
        }
		font[len++] = ' ';
	}
	font[len - 1] = '\0';

    if (last_match_pos)
    {
        char* tmp = strdup(argv[count - 1] + last_match_pos);
        free(argv[count - 1]);
        argv[count - 1] = tmp;
    }

	free(oc->font);

	if (strncmp(font, "pango:", 6) == 0) {
		oc->pango_markup = true;
		oc->font = strdup(font + 6);
		free(font);
	} else {
		oc->pango_markup = false;
		oc->font = font;
	}

	// Parse the font early so we can reject it if it's not valid for pango.
	// Also avoids re-parsing each time we render text.
	PangoFontDescription *font_description = pango_font_description_from_string(oc->font);

	const char *family = pango_font_description_get_family(font_description);
	if (family == NULL) {
		pango_font_description_free(font_description);
		return cmd_results_new(CMD_FAILURE, "Invalid font family.");
	}

	const gint size = pango_font_description_get_size(font_description);
	if (size == 0) {
		pango_font_description_free(font_description);
		return cmd_results_new(CMD_FAILURE, "Invalid font size.");
	}

	if (oc->font_description != NULL) {
		pango_font_description_free(oc->font_description);
	}

	oc->font_description = font_description;

	config->handler_context.leftovers.argc = argc - count;
	config->handler_context.leftovers.argv = argv + count;
	return NULL;
}
