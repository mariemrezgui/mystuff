#include "dotmalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

/*
int main(int argc, char **argv) {
	if (argc < 2) {
		return 1;
	}
	analyse_file(argv[1]);
}
*/

void print_dot_start(FILE *fp) {
	fprintf(fp, "digraph G {\ngraph [splines=\"compound\" pad=\"0.5,0.5\" overlap=\"false\" nodesep=\"0.3\" ];\n");
	fprintf(fp, "node [ shape=\"plain\" ];\nrankdir=\"LR\";\n");
}

void print_dot_end(FILE *fp) {
	fprintf(fp, "}\n");
}

void print_root(chunk_t *root, FILE *fp) {
	fprintf(fp, "root [ label=<<table><tr><td>Root</td></tr><tr><td>Address: %p</td></tr><tr><td>Size: %zu</td></tr><tr><td>File: %d</td></tr><tr><td port=\"l\">List: %p</td></tr></table>> ];\n",
			(void*)root, root->size, root->file, (void*)root->free_space_list);
	if (root->free_space_list != NULL) {
		fprintf(fp, "root:l -> B_%x;\n", (unsigned int) root->free_space_list);
	}
}

void print_block(memory_block_t *block, FILE *fp) {
	unsigned int uid = (unsigned int) block;
	fprintf(fp, "{rank=same; Bl_%x; B_%x;};", uid, uid);
	fprintf(fp, "Bl_%x [ label=<<table><tr><td>memory_block_t *</td></tr><tr><td port=\"p\">%p</td></tr></table>> ];\n", uid, (void*)block);
	fprintf(fp, "B_%x [ label=<<table><tr><td port=\"p\">Prev: <i>%p</i></td></tr><tr><td>Size: <i>%zu</i></td></tr><tr><td port=\"n\">Next: <i>%p</i></td></tr></table>> ];\n",
			uid, (void*)block->previous, block->size, (void*)block->next);

	fprintf(fp, "Bl_%x:p -> B_%x;\n", uid, uid);

}

void print_link(memory_block_t *b1, memory_block_t *b2, FILE *fp) {
	fprintf(fp, "B_%x:n -> B_%x [label=\"N\"];\n", (unsigned int)b1, (unsigned int)b2);
	fprintf(fp, "B_%x:p -> B_%x [label=\"P\"];\n", (unsigned int)b2, (unsigned int)b1);
}

void _analyse_memory(chunk_t *addr, FILE *fp) {
	print_dot_start(fp);
	print_root(addr, fp);

	memory_block_t *next_free = addr->free_space_list;
	while (next_free != NULL) {
		if (next_free->previous != NULL) {
			print_link(next_free->previous, next_free, fp);
		}
		print_block(next_free, fp);
		next_free = next_free->next;
	}

	print_dot_end(fp);
}

int checkpoint_lib_ran = 0;

void checkpoint(const char *name) {

	int has_dot = !system("which dot > /dev/null 2>&1");

	if (has_dot) {
		if (!checkpoint_lib_ran) {
			system("rm -rf png");
			mkdir("png", 0777);
		}

		char namedot[256];
		snprintf(namedot, 256, "%s.dot", name);

		FILE *fpdot = fopen(namedot, "w+");
		_analyse_memory((chunk_t *) START_ADDRESS, fpdot);
		fclose(fpdot);

		char cmddot[1024];
		snprintf(cmddot, 1024, "dot -Tpng -o png/%s.png %s", name, namedot);
		system(cmddot);
		
		remove(namedot);
	}

	if (!checkpoint_lib_ran) {
		system("rm -rf hex");
		mkdir("hex", 0777);
	}
	checkpoint_lib_ran = 1;

	char namehex[256];
	snprintf(namehex, 256, "hex/%s.hex", name);
	
	FILE *fphex = fopen(namehex, "w+");
	for (size_t i = 0; i < get_page_size(); i++) {
		if (i % 24 == 0) {
			fprintf(fphex, "%05zx ", i);
		} 
		fprintf(fphex, "%02x", *(((unsigned char*)START_ADDRESS)+((i/8)*8) + (7-(i%8))));
		if (i % 24 == 23) {
			fprintf(fphex, "\n");
		} else if (i % 8 == 7) {
			fprintf(fphex, " ");
		}
	}
	fclose(fphex);
}

void analyse_memory(chunk_t *addr) {
	_analyse_memory(addr, stdout);
}

void analyse_file(const char *infile) {
	int fd = open(infile, O_RDWR | O_APPEND | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
	struct stat st;
	fstat(fd, &st);
	long int len = st.st_size;
	void *addr = mmap(START_ADDRESS, len, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	analyse_memory((chunk_t*) addr);
}

