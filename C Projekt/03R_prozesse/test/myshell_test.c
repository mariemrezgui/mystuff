#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "plist.h"
#include "shellutils.h"
#include "shellfunctions.h"

#include "../../TestBib/testbib.h"


int main(int argc, char *argv[]) {
	int punkte = 0;
	initExercises(argc, argv);


	// read_input()
	exercise_t *ex1 = addExercise("3.1 Eingabeaufforderung");

	int ret = systemAndFlush("echo 'ls -l' | ./myshell");
	addSubExercise(ex1, "Eingabe", "Teste ob Befehle gelesen werden", verifyIfFileContainsString(ret, "total"), 1, 1);
	ret = systemAndFlush("echo '' | ./myshell"); 
	addSubExercise(ex1, "Beenden", "Teste ob STRG+D die myshell beendet", verifyIfFileContainsString(ret, "Goodbye"), 1, 1);

	// foreground
	punkte = 0;
	exercise_t *ex2 = addExercise("3.2 Vordergrundprozesse");

	ret = systemAndFlush("echo 'ls' | ./myshell");
	verifyIfFileContainsString(ret, "undefined");
	systemAndFlush("echo 'ls -l' | ./myshell");
	systemAndFlush("echo 'ps' | ./myshell");
	systemAndFlush("echo '0' | ./myshell");

	addSubExercise(ex2, "Ausführung", "Testen ob Befehle ausgeführt werden und der exit status korrekt ist.", 0, 0, 5);
	manualExercisePrint("Testen ob Ausführung funktioniert", "(Check der Ausgabe)");

	ret = systemAndFlush("echo 'non_existent_prog' | ./myshell");
	addSubExercise(ex2, "Statusausgabe", "Versuche non_existent_prog auszuführen und überprüfe ob der Exitstatus 1 ist", verifyIfFileContainsString(ret, "non_existent_prog: No such file or directory"), 1, 1);
	

	ret = systemAndFlush("echo 'sleep -x' | ./myshell");
	addSubExercise(ex2, "sleep -x", "sleep -x auszuführen und überprüfe ob der Exitstatus 1 ist", verifyIfFileContainsString(ret, "sleep --help"), 1, 1);

	addSubExercise(ex2, "Fehlerbehandlung", "Überprüfe ob mögliche Fehlerquellen behandelt wurden", 0, 0, 2);	
	manualExercisePrint("Prüfe auf Fehlerbehandlung", "(Check im Quelltext)");



	// background
	punkte = 0;
	exercise_t *ex3 = addExercise("3.3 Hintergrundprozesse");
	//manualExercisePrint("Testen ob Ausführung im Hintergrund funktioniert", "(Check der Ausgabe)");
	systemAndFlush("echo 'sleep 4 &' | ./myshell"); //  >/dev/null 2>&1
	
	ret = systemAndFlush("ps | grep sleep >/dev/null"); // 1p if grep sleep worked
	punkte += !ret?1:0;
	sleep(5);
	ret = systemAndFlush("ps | grep 'sleep <defunct>'"); // 1p if sleep <defunct>
	punkte += !ret?1:0;
	ret = systemAndFlush("ps | grep sleep"); // or not visible at all
	//punkte += ret>1?1:0;
	addSubExercise(ex3, "sleep 4 &", "Überprüfe ob sleep im Hintergrund ausgeführt wird.", 0, 0, 2);
	
		

	// zombies
	punkte = 0;
	exercise_t *ex4 = addExercise("3.4 Zombies");
	char sleep_background [] = "sleep 1 &";
	char ps [] = "ps";
	execute_command(sleep_background);
	sleep(1);
	
	ret = systemAndFlush("ps");
	verifyIfFileContainsString(ret, "undefined");
	collect_defunct_process();
	ret = systemAndFlush("ps");
	callCompletion_t ret_c = synchronousCall("ps");
	// wait until ret_c.pid is done doing stuff
	punkte = sVerifyIfFileContainsString(ret_c, "sleep <defunct>") ? 0 : 2;
	//punkte = verifyIfFileContainsString(ret, "sleep <defunct>") ? 0 : 2;
	
	addSubExercise(ex4, "zombies", "Test ob Zombies erfolgreich eingesammelt werden.", punkte, 2, 2);
	

	// cd
	char go_up[] = "cd ..";
	char go_back[1027];
	char go_root[] = "cd /";
	char go_to_non_existent[] = "cd /non-existing-directory";
	int  dir_len_start, dir_len_up, dir_len_root, dir_len_back;
	char * cwd = malloc(1024);
	punkte = 0;

	bzero(cwd,1024);
    	cwd = getcwd(cwd, 1024);
	bzero(go_back,1027);
	strcpy(go_back, "cd ");
	strcat(go_back, cwd);

	exercise_t *ex5 = addExercise("3.5 Verzeichniswechsel");
	
	bzero(cwd,1024);
	cwd = getcwd(cwd, 1024);
	dir_len_start = strlen(cwd);
	
	execute_command(go_up);
	bzero(cwd,1024);
	cwd = getcwd(cwd, 1024);
	dir_len_up = strlen(cwd);

	execute_command(go_root);
	bzero(cwd,1024);
	cwd = getcwd(cwd, 1024);
	dir_len_root = strlen(cwd);

	execute_command(go_back);
	bzero(cwd,1024);
	cwd = getcwd(cwd, 1024);
	dir_len_back = strlen(cwd);
	
	free(cwd);	
	
	if (dir_len_start > dir_len_up && dir_len_up > dir_len_root && dir_len_root < dir_len_back) {
		punkte += 1;	
	}
	addSubExercise(ex5, "cd", "Prüfe ob der Verzeichniswechsel funktioniert", punkte, 1, 1);
	punkte = 0;
	ret = systemAndFlush("echo 'cd /non-existing-directory' | ./myshell");

	addSubExercise(ex5, "non-existing-directory", "Prüfe ob der Wechsel in 'non-existing-directory' eine Fehlermeldung ausgibt", verifyIfFileContainsString(ret, "cd: No such file or directory"), 1, 1);
	
	//Hintergrundprozesse
	exercise_t *ex6 = addExercise("Zeige Hintergrundprozesse");
	char jobs_command[] = "echo -e \"sleep 4 &\\njobs\" | ./myshell";
	int find = callAndCheck(jobs_command, "sleep 4");

	//verifyIfFileContainsString(ret, "undefined");
	addSubExercise(ex6, "walklist", "Implementiere walklist(), nutze dazu insert und remove element", find * 3, 3, 3);
	manualExercisePrint("Kontrolle der 3.6", "(Check im Quelltext)");

	//Coding Guidelines
	exercise_t *cg = addExercise("Coding Guidelines");
	int coding_guidelines = checkStyle("../../TestBib/checkstyle.sh", "plist.c", "shellfunctions.c", NULL);
	addSubExercise(cg, "Coding Guidelines", "Halte die Coding Guidelines ein", coding_guidelines, 1, 3);
	

	printExerciseTable();
	exercise_score_t* score = passExercises(11);
	return score->returnvalue;
}
