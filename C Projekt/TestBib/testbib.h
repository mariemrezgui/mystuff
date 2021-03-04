#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#define MAX_EXERCISE_NAME 256
#define FILE_MAX_LENGTH (255)
#define BS_SUCCESS "\x1B[32merfolgreich\x1B[0m\n"
#define BS_FAILURE "\x1B[31mfehlgeschlagen\x1B[0m\n"
#define BS_UNKWOWN "\x1B[33munbekannt\x1B[0m\n"
#define OUTPUT_FILE_LOCATION "/tmp"

void manualExercisePrint(char *title, char* target);
void nextExercise();
void additionalInfo(char *info);
void modePrintf(const char *fmt, ...);
int systemAndFlush(const char *cmd);
int verifyIfFileContainsString(int x, const char *to_verify);
int redirectStd(int out_pipe[2], int file_descriptor );
void reconnectStd(int saved_stdout, int file_descriptor);


/* START: Structur-unaware functions */
// Called for marking tasks as manual.
void manualExercisePrint(char* title, char* target) {
    modePrintf("\x1B[30;1;47m[%s]\x1B[0m\x1B[33;1m %s\n\x1B[30;8;1;47m[%s]\x1B[0m \x1B[0m\x1B[33m>> \x1B[0;1m%s\x1B[0m\n\n", "MANUAL", title, "MANUAL", target);
}

// Called internally
void nextExercise() {
	// 55 =
    modePrintf("\x1B[0m\n=======================================================\n\n");
}

// Called to provide further info to error state
void additionalInfo(char *info) {
	modePrintf("%s", info);
}

char * outputLocation(char* suffix) {
	char *username = getenv("USER");
	char alt[] = "ci-user";
	if (username == NULL) {
		username = alt;
	}
	char *fullpath = calloc(FILE_MAX_LENGTH, sizeof(char));
	char folderpath[FILE_MAX_LENGTH];
	snprintf(folderpath, FILE_MAX_LENGTH, "%s/testbib-%s", OUTPUT_FILE_LOCATION, username);
	mkdir(folderpath, S_IRWXU | S_IRWXG | S_IRWXO);

	snprintf(fullpath, FILE_MAX_LENGTH, "%s/%s-%s", folderpath, username, suffix);
	return fullpath;
}
/* END: Structure-unaware functions */

typedef struct sub_exercise {
	char name[MAX_EXERCISE_NAME];
	char description[MAX_EXERCISE_NAME];
	int achieved;
	int achievable;
	int maximal;
} sub_exercise_t;

typedef struct sub_exercise_list {
	sub_exercise_t *sub;
	struct sub_exercise_list *next;
} sub_exercise_list_t;

typedef struct exercise {
	char name[MAX_EXERCISE_NAME];
	int achieved;
	int achievable;
	int maximal;
	sub_exercise_list_t *subs;
} exercise_t;

typedef struct exercise_list {
	exercise_t *exercise;
	struct exercise_list *next;
} exercise_list_t;

typedef struct exercise_score {
	int min; // Mindestpunktzahl notwendig zum Bestehen
	int achieved; // "Sicher" erreichte Punktzahl
	int achievable; // Möglich automatisch erreichbare Punktzahl
	int achievableManual; // Möglich erreichbare Punkte (inkl. Hiwipunkte)
	int maximal; // Maximalpunktzahl
	int returnvalue; // 0: achieved>Min; 1:achievable>Min; 2: sonst
} exercise_score_t;

enum exercise_mode {
	PRINT,
	ENV,
	YAML
};

enum exercise_status {
	RUNNING,
	RESULT
};

// Structure-aware function headers
void initExercises(int argc, char *argv[]);
void checkAchievable(int achieved, int achievable, int maximal);
void printExerciseTitle(char *name);
exercise_t *addExercise(char *name);
void printSubExercise(sub_exercise_t *sub);
sub_exercise_t *addSubExercise(exercise_t* ex, char *name, char *desc, int achieved, int achievable, int maximal);
void walkList(void (*func)(exercise_t*));
exercise_score_t *passExercises(int min);

// Header of exercise list
exercise_list_t *exercises = (exercise_list_t*)NULL;
enum exercise_mode gmode = PRINT;
enum exercise_status gstatus = RUNNING;

void initExercises(int argc, char *argv[]) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--env") == 0) {
			gmode = ENV;
		} else if (strcmp(argv[i], "--yaml") == 0) {
			gmode = YAML;
		}
	}
	srand(time(NULL));
}

void modePrintf(const char *fmt, ...) {
	if (gmode != PRINT) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

int printf(const char *fmt, ...) {
	if (gmode != PRINT && gstatus == RUNNING) {
		return 0;
	}
	va_list args;
	va_start(args, fmt);
	int i = vprintf(fmt, args);
	va_end(args);
	return i;
}

void checkAchievable(int achieved, int achievable, int maximal) {
	if (achieved > achievable || achievable > maximal) {
		modePrintf("Achieved points exceeded achievable points.\n");
	}
}

void printExerciseTitle(char *name) {
	int line = 55;
	int len = strlen(name);
	int equals = line - len - 2;
	int side_equals = equals / 2;
	int left = side_equals;
	int right = side_equals;
	if (equals % 2 == 1) {
		right += 1;
	}
	for (int i = 0; i < left; i++) {
		modePrintf("=");
	}
	modePrintf(" %s ", name);
	for (int i = 0; i < right; i++) {
		modePrintf("=");
	}
	modePrintf("\n\n");
}

exercise_t *addExercise(char *name) {
	printExerciseTitle(name);
	exercise_t *newExercise = malloc(sizeof(exercise_t));
	strncpy(newExercise->name, name, MAX_EXERCISE_NAME);
	newExercise->achieved = 0;
	newExercise->achievable = 0;
	newExercise->maximal = 0;
	newExercise->subs = NULL;

	if (exercises == NULL) {
		exercises = malloc(sizeof(exercise_list_t));
		exercises->next = NULL;
		exercises->exercise = newExercise;
	} else {
		exercise_list_t *last = exercises;
		while (last->next != NULL) {
			last = last->next;
		}
		exercise_list_t *newListEntry = malloc(sizeof(exercise_list_t));
		newListEntry->next = NULL;
		newListEntry->exercise = newExercise;
		last->next = newListEntry;
	}
	return newExercise;
}

void printSubExercise(sub_exercise_t *sub) {
	char erfolg[64];
	if (sub->achievable == 0) {
		strncpy(erfolg, BS_UNKWOWN, 64);
	} else if (sub->achieved == sub->achievable) {
		strncpy(erfolg, BS_SUCCESS, 64);
	} else {
		strncpy(erfolg, BS_FAILURE, 64);
	}
	char *underline = malloc(strlen(sub->name)+1);
	memset(underline, '=', strlen(sub->name));
	underline[strlen(sub->name)]='\0';
	char manualNote[MAX_EXERCISE_NAME] = "";
	if (sub->achievable != sub->maximal) {
		snprintf(manualNote, MAX_EXERCISE_NAME, " (%d Punkt(e) werden zusätzlich von Hiwis vergeben)", sub->maximal-sub->achievable);
	}
	modePrintf("%2s%s\n%2s%s\n\n %s: %s Vergebene Punkte: %d/%d%s\n\n", "", sub->name, "", underline, sub->description, erfolg,
			sub->achieved, sub->achievable, manualNote);
	free(underline);
}

sub_exercise_t *addSubExercise(exercise_t* ex, char *name, char *desc, int achieved, int achievable, int maximal) {
	checkAchievable(achieved, achievable, maximal);
	if (ex->achieved != 0 && ex->subs == NULL) {
		modePrintf("Exercises with points can't have sub exercises. Set exercise points to zero and use sub exercises for all points instead.\n");
	}
	sub_exercise_t *sub = malloc(sizeof(sub_exercise_t));
	strncpy(sub->name, name, MAX_EXERCISE_NAME);
	strncpy(sub->description, desc, MAX_EXERCISE_NAME);
	sub->achieved = achieved;
	sub->achievable = achievable;
	sub->maximal = maximal;
	printSubExercise(sub);
	ex->achieved += achieved;
	ex->achievable += achievable;
	ex->maximal += maximal;

	if (ex->subs == NULL) {
		ex->subs = malloc(sizeof(sub_exercise_list_t));
		ex->subs->next = NULL;
		ex->subs->sub = sub;
	} else {
		sub_exercise_list_t *last = ex->subs;
		while (last->next != NULL) {
			last = last->next;
		}
		sub_exercise_list_t *newSubEntry = malloc(sizeof(sub_exercise_list_t));
		newSubEntry->next = NULL;
		newSubEntry->sub = sub;
		last->next = newSubEntry;
	}
	return sub;
}

void walkList(void (*func)(exercise_t*)) {
	exercise_list_t *last = exercises;
	if (exercises != NULL) {
		func(exercises->exercise);
	}
	while (last != NULL && last->next != NULL) {
		last = last->next;
		func(last->exercise);
	}
}

void printExercise(exercise_t *exercise) {
	modePrintf("%s: %d/%d max:%d\n", exercise->name,
			exercise->achieved, exercise->achievable, exercise->maximal);
	sub_exercise_list_t *subs = exercise->subs;
	while (subs != NULL) {
		modePrintf(" - ");
		printSubExercise(subs->sub);
		subs = subs->next;
	}
}

void printExerciseTable() {
	if (gmode != PRINT) {
		return;
	}
	modePrintf("----- ÜBERSICHT -----\n");
	exercise_list_t *last = exercises;
	while (last != NULL) {
		exercise_t *ex = last->exercise;
		modePrintf("%s: %d/%d (max: %d)\n", ex->name, ex->achieved, ex->achievable, ex->maximal);
		sub_exercise_list_t *subs = ex->subs;
		while (subs != NULL) {
			modePrintf(" - %s: %d/%d\n", subs->sub->name, subs->sub->achieved, subs->sub->achievable);
			subs = subs->next;
		}
		last = last->next;
	}
	exercise_score_t *score = passExercises(0);
	modePrintf("\nerreicht: %d/%d, erreichbar: %d/%d\n", score->achieved, score->achievable, score->achievableManual, score->maximal);
	modePrintf("also sind noch %d Punkte von Hiwis zu vergeben.\n", score->maximal - score->achievable);
}


void printExerciseList() {
	walkList(&printExercise);
}

void envExerciseList() {
	// TODO
}

void validateExercise(exercise_t *ex) {
	if (ex->subs == NULL) {
		return;
	}
	int sum_achievable = 0;
	sub_exercise_list_t *subs = ex->subs;
	while (subs != NULL) {
		sum_achievable += subs->sub->achievable;
		subs = subs->next;
	}
	if (sum_achievable != ex->achievable) {
		modePrintf("Achievable point of exercise and sub exercise list don't match!\n");
	}
}

void exercise_prints_yaml(char* node, char* content, int indent) {
	for (int i=0; i<indent; i++) {
		printf("  ");
	}
	printf("%s: \"%s\"\n", node, content);
}

void exercise_printi_yaml(char* node, int content, int indent) {
	for (int i=0; i<indent; i++) {
		printf("  ");
	}
	printf("%s: %d\n", node, content);
}

exercise_score_t *passExercises(int min) {
	walkList(&validateExercise);
	gstatus = RESULT;
	exercise_list_t *last = exercises;
	int sum_achieved = 0;
	int sum_achievable = 0;
	int sum_achievableManual = 0;
	int sum_maximal = 0;
	while (last != NULL) {
		exercise_t *ex = last->exercise;
		sum_achieved += ex->achieved;
		sum_achievable += ex->achievable;
		sum_achievableManual += ex->maximal - (ex->achievable - ex->achieved);
		sum_maximal += ex->maximal;
		last = last->next;
	}
	exercise_score_t *score = malloc(sizeof(exercise_score_t));
	score->min = min;
	score->achieved = sum_achieved;
	score->achievable = sum_achievable;
	score->achievableManual = sum_achievableManual;
	score->maximal = sum_maximal;
	if (sum_achieved >= min) {
		score->returnvalue = 0;
	} else if (sum_achievable >= min) {
		score->returnvalue = 1;
	} else {
		score->returnvalue = 2;
	}
	if (gmode == ENV) {
		printf("BSENVTOKENSTART\n");
		printf("CI_RUNNER_ID='%s'\n", getenv("CI_RUNNER_ID"));
		printf("CI_COMMIT_SHA='%s'\n", getenv("CI_COMMIT_SHA"));
		printf("CI_JOB_NAME='%s'\n", getenv("CI_JOB_NAME"));
		printf("CI_JOB_STAGE='%s'\n", getenv("CI_JOB_STAGE"));
		printf("CI_PIPELINE_ID='%s'\n", getenv("CI_PIPELINE_ID"));
		printf("CI_REPOSITORY_URL='%s'\n", getenv("CI_REPOSITORY_URL"));
		printf("GITLAB_USER_ID='%s'\n", getenv("GITLAB_USER_ID"));
		printf("GITLAB_USER_EMAIL='%s'\n", getenv("GITLAB_USER_EMAIL"));
		printf("EXERCISE_MIN=%d\n", score->min);
		printf("EXERCISE_PASS=%d\n", score->returnvalue);
		printf("EXERCISE_ACHIEVED=%d\n", score->achieved);
		printf("EXERCISE_ACHIEVABLE=%d\n", score->achievable);
		printf("EXERCISE_ACHIEVABLE_MANUAL=%d\n", score->achievableManual);
		printf("EXERCISE_MANUAL=%d\n", score->achievableManual - score->achievable);
		printf("EXERCISE_MAX=%d\n", score->maximal);
		last = exercises;
		for (int i = 1; last != NULL; i++) {
			printf("EXERCISE_%d_ACHIEVED=%d\n", i, last->exercise->achieved);
			printf("EXERCISE_%d_NAME=\"%s\"\n", i, last->exercise->name);
			printf("EXERCISE_%d_ACHIEVABLE=%d\n", i, last->exercise->achievable);
			printf("EXERCISE_%d_MAX=%d\n", i, last->exercise->maximal);
			sub_exercise_list_t *subs = last->exercise->subs;
			for (int j = 1; subs != NULL; j++) {
				printf("EXERCISE_%d_%d_ACHIEVED=%d\n", i, j, subs->sub->achieved);
				printf("EXERCISE_%d_%d_ACHIEVABLE=%d\n", i, j, subs->sub->achievable);
				printf("EXERCISE_%d_%d_MAX=%d\n", i, j, subs->sub->maximal);
				printf("EXERCISE_%d_%d_TITLE=\"%s\"\n", i, j, subs->sub->name);
				printf("EXERCISE_%d_%d_DESC=\"%s\"\n", i, j, subs->sub->description);
				subs = subs->next;
			}
			last = last->next;
		}
	} else if (gmode == YAML) {
		printf("BSYAMLTOKENSTART\n");
		exercise_prints_yaml("runner_id", getenv("CI_RUNNER_ID"), 0);
		exercise_prints_yaml("commit", getenv("CI_COMMIT_SHA"), 0);
		exercise_prints_yaml("job_name", getenv("CI_JOB_NAME"), 0);
		exercise_prints_yaml("job_stage", getenv("CI_JOB_STAGE"), 0);
		exercise_prints_yaml("pipeline", getenv("CI_PIPELINE_ID"), 0);
		exercise_prints_yaml("repo", getenv("CI_REPOSITORY_URL"), 0);
		exercise_prints_yaml("user_id", getenv("GITLAB_USER_ID"), 0);
		exercise_prints_yaml("user_email", getenv("GITLAB_USER_EMAIL"), 0);
		exercise_printi_yaml("min", score->min, 0);
		exercise_printi_yaml("pass", score->returnvalue, 0);
		exercise_printi_yaml("achieved", score->achieved, 0);
		exercise_printi_yaml("achievable", score->achievable, 0);
		exercise_printi_yaml("achievableManual", score->achievableManual, 0);
		exercise_printi_yaml("manual", score->achievableManual - score->achievable, 0);
		exercise_printi_yaml("max", score->maximal, 0);
		last = exercises;
		printf("exercises:\n");
		for (int i = 1; last != NULL; i++) {
			printf("  exercise_%d:\n", i); //indent=1
			exercise_prints_yaml("title", last->exercise->name, 2);
			exercise_printi_yaml("achieved", last->exercise->achieved, 2);
			exercise_printi_yaml("achievable", last->exercise->achievable, 2);
			exercise_printi_yaml("max", last->exercise->maximal, 2);
			sub_exercise_list_t *subs = last->exercise->subs;
			printf("    subs:\n"); // indent=2
			for (int j = 1; subs != NULL; j++) {
				printf("      exercise_%d_%d:\n", i, j); // indent=3
				exercise_prints_yaml("title", subs->sub->name, 4);
				exercise_prints_yaml("description", subs->sub->description, 4);
				exercise_printi_yaml("achieved", subs->sub->achieved, 4);
				exercise_printi_yaml("achievable", subs->sub->achievable, 4);
				exercise_printi_yaml("max", subs->sub->maximal, 4);
				subs = subs->next;
			}
			last = last->next;
		}
		
	}
	return score;
}

void flushAll() {
	fflush(stdout);
	fflush(stderr);
}


void codeStart() {
	if (gmode == PRINT) {
		flushAll();
		printf("```\n");
		flushAll();
	} else {
	}
}

void codeEnd() {
	if (gmode == PRINT) {
		flushAll();
		printf("```\n\n");
		flushAll();
	} else {
	}
}

typedef struct callCompletion {
	char filename[FILE_MAX_LENGTH];
	char exitfile[FILE_MAX_LENGTH];
	int pid;
	int random;
} callCompletion_t;

int isCallComplete(callCompletion_t call) {
	int ret = kill(call.pid, 0);
	return ret == 0 ? 0 : 1;
}

void waitFor(callCompletion_t call) {
	while (!isCallComplete(call)) {
		usleep(100 * 1000);
	}
}

// Waits on call, timeout is in seconds
void waitUntil(callCompletion_t call, int timeout) {
	for (int i = 0; i < timeout * 10; i++) {
		if (isCallComplete(call)) {
			return;
		}
		usleep(100 * 1000);
	}
}

int getReturnValue(callCompletion_t call) {
	waitFor(call);
	FILE *file = fopen(call.exitfile, "r");
	if (file == NULL) {
		printf("Opening exitfile: %s failed\n", call.exitfile);
	}
	int exitvalue;
	fscanf(file, "%d", &exitvalue);
	return exitvalue;
}

int systemAndFlush(const char *cmd) {
	int x = rand();
	char command[2*FILE_MAX_LENGTH];

	char suffix[FILE_MAX_LENGTH];
	snprintf(suffix, FILE_MAX_LENGTH, "%d.txt", x);
	char * filename = outputLocation(suffix);

	snprintf(command, 2*FILE_MAX_LENGTH, "%s > %s %s", cmd, filename, "2>&1");
	system(command);

	free(filename);
	return x;
}

callCompletion_t parallelCall(const char *cmd) {
	int x = rand();
	char command[4*FILE_MAX_LENGTH];
	char *filename;
	char *exitfile;
	char *callfile;

	char outSuffix[FILE_MAX_LENGTH];
	char exitSuffix[FILE_MAX_LENGTH];
	char callSuffix[FILE_MAX_LENGTH];

	snprintf(outSuffix, FILE_MAX_LENGTH, "%d.txt", x);
	snprintf(exitSuffix, FILE_MAX_LENGTH, "%d-exit.txt", x);
	snprintf(callSuffix, FILE_MAX_LENGTH, "%d-call.txt", x);

	filename = outputLocation(outSuffix);
	exitfile = outputLocation(exitSuffix);
	callfile = outputLocation(callSuffix);

	snprintf(command, 4*FILE_MAX_LENGTH, "bash -c '(%s >%s 2>&1; echo $? > %s) &:; echo \"$!-%d\" > %s'", cmd, filename, exitfile, x, callfile);

	system(command);

	FILE *file = fopen(callfile, "r");
	if (file == NULL) {
		printf("Opening %s didn't work\n", callfile);
	}

	int pid;
	int x_read;

	int match = fscanf(file, "%d-%d", &pid, &x_read);
	if (match != 2) {
		printf("Scanning %s went wrong\n", callfile);
	}

	if (x_read != x) {
		printf("Random values written and read don't match\n");
	}

	fclose(file);

	callCompletion_t ret;
	ret.pid = pid;
	strncpy(ret.filename, filename, FILE_MAX_LENGTH);
	strncpy(ret.exitfile, exitfile, FILE_MAX_LENGTH);
	ret.random = x;

	free(filename);
	free(exitfile);
	free(callfile);
	return ret;
}

//
// Calls `cmd` creating a callCompletion_t, and wait()s on it.
// Then returns the call object.
//
callCompletion_t synchronousCall(const char *cmd) {
	callCompletion_t call = parallelCall(cmd);
	waitUntil(call, 600);
	return call;
}

int sVerifyIfFileContainsString(callCompletion_t call, const char *to_verify) {
        size_t length, read;
        int found = 0;
        char* line = NULL;
        FILE *file;
        file = fopen(call.filename,"r");
        if (file == NULL) {
                return 0;
        }
        codeStart();
        while((read = getline(&line, &length, file)) != (size_t)-1) {

                printf("%s", line);

                if (strstr(line, to_verify) != NULL) {
                    found = 1;
                }
        }
        codeEnd();
		fclose(file);
        return found;
}

int verifyIfFileContainsString(int x, const char *to_verify) {
		char ext[FILE_MAX_LENGTH];
		snprintf(ext, FILE_MAX_LENGTH, "%d.txt", x);
        char * filename = outputLocation(ext);
		callCompletion_t call;
		call.pid = 0;
		strcpy(call.filename, filename);
		free(filename);
		return sVerifyIfFileContainsString(call, to_verify);
}

int callAndCheck(const char *cmd, const char *substring) {
	callCompletion_t call = synchronousCall(cmd);
	return sVerifyIfFileContainsString(call, substring);
}

// Returns 1 if no errors found, end arg list with NULL
int checkStyle(const char *checkstyle, ...) {
	va_list args;
	va_start(args, checkstyle);
	
	char *current = va_arg(args, char*);
	char command[FILE_MAX_LENGTH];
	while (current != NULL) {
		snprintf(command, FILE_MAX_LENGTH, "bash %s %s", checkstyle, current);
		callCompletion_t check = synchronousCall(command);

		modePrintf("Checking %s for coding guidelines: ", current);

		if (getReturnValue(check) != 0) {
			modePrintf("❌\n\n");
			return 0;
		}

		modePrintf("✔️\n");

		current = va_arg(args, char*);
	}
	modePrintf("\n");
	va_end(args);
	return 1;
}


int redirectStd(int out_pipe[2], int file_descriptor ) {
        // file_descriptor = 1 -> STDOUT_FILENO
        // file_descriptor = 2 -> STDERR_FILENO
        int saved_stdout = dup(file_descriptor);

        if (pipe(out_pipe) != 0) {
                exit(1);
        }

        dup2(out_pipe[1], file_descriptor);   //redirect stdout to the pipe
        close(out_pipe[1]);
        return saved_stdout;
}

void reconnectStd(int saved_stdout, int file_descriptor) {
        // file_descriptor = 1 -> STDOUT_FILENO
        // file_descriptor = 2 -> STDERR_FILENO
        dup2(saved_stdout, file_descriptor);  // reconnect pipe
}
