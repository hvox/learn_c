#include <stdio.h>
#include <string.h>
#include <dirent.h>

const int tab_size = 2;

void print_dir(const char *path, int indent) {
	DIR *dir = opendir(path);
	if (dir == NULL) return; // fail to open directory
	struct dirent *entry; // this line can be moved down
	while (entry = readdir(dir)) {
		if (entry->d_type != DT_DIR) {
			printf("%*s- %s\n", indent * tab_size, "", entry->d_name);
			continue;
		}
		if (strcmp(entry->d_name, ".") == 0) continue;
		if (strcmp(entry->d_name, "..") == 0) continue;
		printf("%*s+ %s/\n", indent * tab_size, "", entry->d_name);
		char entry_path[1024];
		snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);
		print_dir(entry_path, indent + 1);
	}
	closedir(dir);
}

int main(void) {
	print_dir(".", 0);
	return 0;
}
