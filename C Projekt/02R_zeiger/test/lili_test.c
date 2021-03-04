#include <time.h>
#include <stdlib.h>
#include "lili.h"
#include "../../TestBib/testbib.h"

extern element_t* head;

int main(int argc, char *argv[])
{
	int punkte =0;
	initExercises(argc, argv);

	srand(time(NULL));

	// Vorbereitung
	exercise_t *ex1 = addExercise("2.1 Vorbereitung");
	addSubExercise(ex1, "Erste Schritte", "Befolge die Anleitung", 1, 1, 1);

	// insert_element()
	exercise_t *ex2 = addExercise("2.2 insert_element()");

	unsigned int e1 = rand();
	insert_element(e1);
	addSubExercise(ex2, "insert_element() ausführen", "Der Test führt insert_element aus", 1, 1, 1);

	if (head->data == e1)
	{
		punkte = 1;
	}
	else
	{
		punkte = 0;
	}
	addSubExercise(ex2, "insert_element() benutzen", "Der Test fügt ein Element hinzu", punkte, 1, 1);

	unsigned int e2 = rand();
	insert_element(e2);
	if (head->data == e1 && head->next != NULL && head->next->data == e2)
	{
		punkte = 1;
	}
	else
	{
		punkte = 0;
	}
	addSubExercise(ex2, "insert_element() zweites Element", "Der Test fügt ein weiteres Element hinzu", punkte, 1, 1);

	punkte = 0;
	unsigned int e3, e4 = rand();
	insert_element(e3);
	insert_element(e4);
	unsigned int werte[] = {e1, e2, e3, e4};
	int insert_t_fail = 0;
	element_t* insert_tester = head;
	for (int i = 0; i < 4; i++)
	{
		if (insert_tester->data != werte[i])
		{
			additionalInfo("Unerwarteter Listeninhalt.\n");
			insert_t_fail = 1;
			break;
		}
		if (i < 3 && insert_tester->next == NULL)
		{
			additionalInfo("Liste zu kurz.\n");
			insert_t_fail = 1;
			break;
		}
		insert_tester = insert_tester->next;
	}
	if (!insert_t_fail)
	{
		punkte = 2;
	}
	addSubExercise(ex2, "insert_element() weitere Elemente", "Der Test fügt einige Elemente hinzu", punkte, 2, 2);

	addSubExercise(ex2, "Perror", "Behandle alle Fehlerzustände", 0, 0, 1);
	manualExercisePrint("Testen auf Benutzung von perror per Hand", "(perror Check im Quelltext)");

	// print_lili
	exercise_t *ex3 = addExercise("2.3 print_lili()");
	head = NULL;

	codeStart();
	print_lili();
	codeEnd();
	addSubExercise(ex3, "print_lili() ausführen", "Der Test führt print_lili() aus", 1, 1, 1);

	insert_element(1);
	insert_element(2);
	insert_element(3);
	insert_element(4);
	insert_element(5);
	insert_element(6);
	addSubExercise(ex3, "print_lili() Kontrolle", "Prüfe ob print_lili() die Elemente in der richtigen Reihenfolge ausgibt", 0, 0, 2);
	manualExercisePrint("Kontrolliere print_lili() auf die richtige Reihenfolge", "1, 2, 3, 4, 5, 6,");
	codeStart();
	print_lili();
	codeEnd();

	// remove_element()
	exercise_t *ex4 = addExercise("2.4 remove_element()");
	remove_element();
	addSubExercise(ex4, "remove_element() ausführen #1", "Der Test führt remove_element() aus", 1, 1, 1);

	addSubExercise(ex4, "Memoryleaks", "Habe keine Memoryleaks", 0, 0, 1);
	manualExercisePrint("Prüfe auf Memoryleaks", "(Check im Quelltext)");

	punkte = 0;
	head = NULL;
	codeStart();
	remove_element();
	codeEnd();
	if (head == NULL)
	{
		punkte = 1;
	}
	addSubExercise(ex4, "remove_element() ausführen (#2)", "Der Test prüft auf das Verhalten bei leerer Liste", punkte, 1, 1);

	punkte = 0;
	head = NULL;
	unsigned int remove_werte[] = {rand(), rand(), rand(), rand(), rand()};
	int failed = 0;
	for (int i = 0; i < 5; i++)
	{
		insert_element(remove_werte[i]);
	}
	for (int i = 0; i < 5; i++)
	{
		unsigned int ret = remove_element();
		if (ret != remove_werte[i])
		{
			additionalInfo("Falscher Rückgabewert.\n");
			failed = 1;
			break;
		}
		element_t* rem_iter = head;
		for (int j = i+1; j < 5; j++)
		{
			if (rem_iter == NULL || rem_iter->data != remove_werte[j])
			{
				additionalInfo("Werte falsch entfernt.\n");
				failed = 1;
				break;
			}
			rem_iter = rem_iter->next;
		}
	}
	if (head != NULL)
	{
		additionalInfo("Liste wurde nicht geleert.\n");
		failed = 1;
	}
	if (!failed)
	{
		punkte = 2;
	}
	addSubExercise(ex4, "remove_element() testen", "Teste Funktionsweise und Rückgabewerte", punkte, 2, 2);

	exercise_t *cg = addExercise("Coding Guidelines");
	addSubExercise(cg, "Coding Guidelines", "Halte die Coding Guidelines ein", 0, 0, 3);

	printExerciseTable();
	exercise_score_t* score = passExercises(9);
	return score->returnvalue;

}
