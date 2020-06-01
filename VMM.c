#include <stdio.h>
#include <malloc.h>
#pragma warning(disable:4996)

int N; //process' # of page
int M; //ONLY IN FA - # of allocated page frame
int W; //ONLY IN VA (Working Set) - Window Size
int K; //length of page reference

int print_result_FA(int** arr);
int print_result_VA(int** arr);

int main() {
	
	int time;

	scanf("%d %d %d %d", &N, &M, &W, &K);
	int* rstring = (int*)malloc(sizeof(int) * K);

	for (int i = 0; i < K; i++) {
		scanf("%d", rstring+i);
	}

	int** MIN = malloc(sizeof(int*) * K);
	int** FIFO = malloc(sizeof(int*) * K);
	int** LRU = malloc(sizeof(int*) * K);
	int** LFU = malloc(sizeof(int*) * K);
	int** WS = malloc(sizeof(int*) * K);

	for (int i = 0; i < K; i++) {
		MIN[i] = malloc(sizeof(int) * (M + 1));
		FIFO[i] = malloc(sizeof(int) * (M + 1));
		LRU[i] = malloc(sizeof(int) * (M + 1));
		LFU[i] = malloc(sizeof(int) * (M + 1));
		WS[i] = malloc(sizeof(int) * (N + 1));
	}

	//init arrays with -1
	for (int i = 0; i < K; i++) {
		for (int j = 0; j < M + 1; j++) {
			MIN[i][j] = -1;
			FIFO[i][j] = -1;
			LRU[i][j] = -1;
			LFU[i][j] = -1;
		}
		for (int j = 0; j < N + 1; j++) {
			WS[i][j] = -1;
		}
	}

	/*
	1. MIN Algorithm
	*/
	int empty_flag;
	time = 0;
	int* status = malloc(sizeof(int) * N); //Shows if Nth page is on memory. 1/0
	for (int i = 0; i < N; i++) {status[i] = 0;} //init with 0 
	int newpage; 
	while (time < K) {
		newpage = rstring[time];
		switch (status[newpage]){
		case 0: //Page Fault
			empty_flag = 0;
			for (int i = 0; i < M; i++) { //Case 1. When there's empty memory space
				if (MIN[time][i] == -1) {
					MIN[time][i] = newpage;
					empty_flag = 1;
					break;
				}
			}
			if (empty_flag == 0) { //Case 2. There's no empty memory space - Manegement needed
				int find_flag;
				int max_forward_distance = 0;
				int temp_idx = 0;
				for (int i = 0; i < M && MIN[time][i] != -1; i++) { //find Maximum Df(page#) and idx of that page
					find_flag = 0;
					for (int j = time + 1; j < K; j++) {
						if (rstring[j] == MIN[time][i]) {
							find_flag = 1;
							if (max_forward_distance < j - time) {
								max_forward_distance = j - time;
								temp_idx = i;
							}
							break;
						}
					}
					if (find_flag == 0) { //this page will never be referenced. TIE BREAKING RULE: Page with smallest page number
						temp_idx = i;
						max_forward_distance = 9999;
						break;
					}
				}
				status[MIN[time][temp_idx]] = 0;
				MIN[time][temp_idx] = newpage;
			}
			status[newpage] = 1;
			MIN[time][M] = 1; //Set PageFaultFlag to 1
			break;
		case 1:
			MIN[time][M] = 0; //Set PageFaultFlag to 0
			break;
		}
		for (int i = 0; i < M && time + 1 != K; i++) {
			MIN[time + 1][i] = MIN[time][i]; //Set record of 1 Second later in advance. 코드의 보편성을 위해서
		}
		time++;
	}

	/*
	2. FIFO Algorithm
	*/
	time = 0; 
	int* timestamp = malloc(sizeof(int) * N); //Saves TimeStamp (Load Time)
	for (int i = 0; i < N; i++) { timestamp[i] = -1; } //init with -1
	for (int i = 0; i < N; i++) { status[i] = 0; } //init with 0 
	while (time < K) {
		newpage = rstring[time];
		switch (status[newpage]) {
		case 0: //Page Fault
			empty_flag = 0;
			for (int i = 0; i < M; i++) { //Case 1. When there's empty memory space
				if (FIFO[time][i] == -1) {
					FIFO[time][i] = newpage;
					empty_flag = 1;
					break;
				}
			}
			if (empty_flag == 0) { //Case 2. There's no empty memory space - Manegement needed
				int min_load_time = timestamp[FIFO[time][0]];
				int temp_idx = 0;
				for (int i = 0; i < M && FIFO[time][i] != -1; i++) { //find page which is now loaded on memory & timestamp is minimum 
					if (min_load_time >= timestamp[FIFO[time][i]]) {
						min_load_time = timestamp[FIFO[time][i]];
						temp_idx = i;
					}
				}
				status[FIFO[time][temp_idx]] = 0;
				FIFO[time][temp_idx] = newpage;
			}
			status[newpage] = 1;
			timestamp[newpage] = time;
			FIFO[time][M] = 1; //Set PageFaultFlag to 1
			break;
		case 1:
			FIFO[time][M] = 0; //Set PageFaultFlag to 0
			break;
		}
		for (int i = 0; i < M && time + 1 != K; i++) {
			FIFO[time + 1][i] = FIFO[time][i]; //Set record of 1 Second later in advance. 코드의 보편성을 위해서
		}
		time++;
	}


	/*
	3. LRU Algorithm
	*/
	time = 0;
	for (int i = 0; i < N; i++) { timestamp[i] = -1; } //Saves TimeStamp (Referenced Time) //init with -1
	for (int i = 0; i < N; i++) { status[i] = 0; } //init with 0 
	while (time < K) {
		newpage = rstring[time];
		switch (status[newpage]) {
		case 0: //Page Fault
			empty_flag = 0;
			for (int i = 0; i < M; i++) { //Case 1. When there's empty memory space
				if (LRU[time][i] == -1) {
					LRU[time][i] = newpage;
					empty_flag = 1;
					break;
				}
			}
			if (empty_flag == 0) { //Case 2. There's no empty memory space - Manegement needed
				int min_ref_time = timestamp[LRU[time][0]];
				int temp_idx = 0;
				for (int i = 1; i < M && LRU[time][i] != -1; i++) { //find page which is now loaded on memory & timestamp is minimum 
					if (min_ref_time >= timestamp[LRU[time][i]]) {
						min_ref_time = timestamp[LRU[time][i]];
						temp_idx = i;
					}
				}
				status[LRU[time][temp_idx]] = 0;
				LRU[time][temp_idx] = newpage;
			}
			status[newpage] = 1;
			LRU[time][M] = 1; //Set PageFaultFlag to 1
			break;
		case 1:
			LRU[time][M] = 0; //Set PageFaultFlag to 0
			break;
		}
		timestamp[newpage] = time;
		for (int i = 0; i < M && time + 1 != K; i++) {
			LRU[time + 1][i] = LRU[time][i]; //Set record of 1 Second later in advance. 코드의 보편성을 위해서
		}
		time++;
	}

	/*
	4. LFU Algorithm
	*/
	time = 0;
	int* refcount = (int*)malloc(sizeof(int) * N); //Saves # of count 
	for (int i = 0; i < N; i++) { refcount[i] = 0; } //init with 0
	for (int i = 0; i < N; i++) { timestamp[i] = -1; } //Saves TimeStamp (Referenced Time) //init with -1 
	for (int i = 0; i < N; i++) { status[i] = 0; } //init with 0 
	while (time < K) {
		newpage = rstring[time];
		refcount[newpage]++;
		switch (status[newpage]) {
		case 0: //Page Fault
			empty_flag = 0;
			for (int i = 0; i < M; i++) { //Case 1. When there's empty memory space
				if (LFU[time][i] == -1) {
					LFU[time][i] = newpage;
					empty_flag = 1;
					break;
				}
			}
			if (empty_flag == 0) { //Case 2. There's no empty memory space - Manegement needed
				int min_ref_count = refcount[LFU[time][0]];
				int tie_breaking_flag = 0;
				int temp_idx = 0;
				for (int i = 1; i < M && LFU[time][i] != -1; i++) { //find page which is now loaded on memory & timestamp is minimum 					
					if (min_ref_count > refcount[LFU[time][i]]) {
						min_ref_count = refcount[LFU[time][i]];
						temp_idx = i;
					}
					else if (min_ref_count == refcount[LFU[time][i]] && timestamp[LFU[time][temp_idx]] > timestamp[LFU[time][i]]) { //Apply tie breaking rule - LRU. 
						temp_idx = i;
					}
				}
				status[LFU[time][temp_idx]] = 0;
				LFU[time][temp_idx] = newpage;
			}
			status[newpage] = 1;
			LFU[time][M] = 1; //Set PageFaultFlag to 1
			break;
		case 1:
			LFU[time][M] = 0; //Set PageFaultFlag to 0
			break;
		}
		timestamp[newpage] = time;
		for (int i = 0; i < M && time + 1 != K; i++) {
			LFU[time + 1][i] = LFU[time][i]; //Set record of 1 Second later in advance. 코드의 보편성을 위해서
		}
		time++;
	}


	/*
	5. Working Set Algorithm
	*/
	time = 0;
	int* workingset = (int*)malloc(sizeof(int) * N); //Saves working set - If page[n] is in working set -> workingset[n]==1. 
	while (time < K) {
		for (int i = 0; i < N; i++) { workingset[i] = 0; } //init with 0
		newpage = rstring[time];
		switch (WS[time][newpage]) {
		case -1: //Page Fault
			WS[time][N] = 1;
			break;
		case 1:
			WS[time][N] = 0;
			break;
		}
		WS[time][newpage] = 1;
		for (int i = 0; i <= W && time - i >= 0; i++) {
			workingset[rstring[time - i]] = 1;
		}
		if (time - W - 1 >= 0 && workingset[rstring[time - W - 1]] == 0) //Let the page which is not in working set any more come out from memory
			WS[time][rstring[time - W - 1]] = -1;
		for (int i = 0; i < N && time + 1 != K; i++) {
			WS[time + 1][i] = WS[time][i]; //Set record of 1 Second later in advance. 코드의 보편성을 위해서
		}
		time++;
	}


	puts("\n\n");
	printf("************************\n");
	printf("* Result of MIN Method *\n");
	printf("************************\n");
	print_result_FA(MIN);

	printf("*************************\n");
	printf("* Result of FIFO Method *\n");
	printf("*************************\n");
	print_result_FA(FIFO);

	printf("************************\n");
	printf("* Result of LRU Method *\n");
	printf("************************\n");
	print_result_FA(LRU);

	printf("************************\n");
	printf("* Result of LFU Method *\n");
	printf("************************\n");
	print_result_FA(LFU);

	printf("********************************\n");
	printf("* Result of Working Set Method *\n");
	printf("********************************\n");
	print_result_VA(WS);
}

int print_result_FA(int** arr) {
	int value, count = 0;
	for (int i = 0; i < K; i++) {
		if (arr[i][M] == 1)
			count++;
	}
	printf("Number of Page Fault: %d\n", count); 
	printf("-----");
	for (int i = 0; i < M; i++)
		printf("----", i);
	printf("------------\n");
	printf("Time|");
	for (int i = 2 * M - 6; i > 0; i--)
		printf(" ");
	printf("Memory State");
	for (int i = 2 * M - 6; i > 0; i--)
		printf(" ");
	printf("| Page Fault\n");
	printf("-----");
	for (int i = 0; i < M; i++)
		printf("----", i);
	printf("------------\n");
	for (int i = 0; i < K; i++) {
		printf("%4d|", i + 1);
		for (int j = 0; j < M; j++) {
			value = arr[i][j];
			if (value == -1)
				printf("    ");
			else
				printf("%4d", value);
		}
		if (arr[i][M] == 1)
			printf("|    F\n");
		else
			puts("|");
	}
	puts("");
}


int print_result_VA(int** arr) {
	int value, count = 0;
	for (int i = 0; i < K; i++) {
		if (arr[i][N] == 1)
			count++;
	}
	printf("Number of Page Fault: %d\n", count);
	printf("-----");
	for (int i = 0; i < N; i++)
		printf("----", i);
	printf("------------\n");
	printf("Time|");
	for (int i = 0; i < N; i++)
		printf("%4d", i);
	printf("| Page Fault\n");
	printf("-----");
	for (int i = 0; i < N; i++)
		printf("----", i);
	printf("------------\n");
	for (int i = 0; i < K; i++) {
		printf("%4d|", i + 1); //time
		for (int j = 0; j < N; j++) {
			value = arr[i][j];
			if (value == -1)
				printf("    ");
			else
				printf("%4d", j);
		}
		if (arr[i][N] == 1)
			printf("|    F\n");
		else
			puts("|");
	}
	puts("");
}
