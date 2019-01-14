#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <windows.h>
#define MAX_SIZE 1100

using namespace std;

int numberOfNodes, filecounter = 0, coords[MAX_SIZE][2];
static double graph[MAX_SIZE][MAX_SIZE];
string filename;

double calcDistance(int x1, int y1, int x2, int y2) {
	return sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2));
}

void createInstance() {

	string filename;
	int numberOfNodes;
	ofstream destination;

	cout << "Podaj nazwe pliku" << endl;
	cin >> filename;
	cout << "Podaj ilosc miast do odwiedzenia" << endl;
	cin >> numberOfNodes;

	destination.open(filename + ".txt");
	destination << numberOfNodes << endl;

	for (int i = 0; i < numberOfNodes; i++) {
		destination << i + 1 << " " << rand() % 500 + 1 << " " << rand() % 500 + 1 << endl;
	}

	destination.close();
}

void readInstance() {

	int buffer;
	ifstream source;
	source.open(filename + ".txt");
	source >> numberOfNodes;

	//cout << numberOfNodes << endl;
	for (int i = 0; i < numberOfNodes; i++) {
		source >> buffer;
		source >> coords[i][0] >> coords[i][1];
		//cout << coords[i][0] << " " << coords[i][1] << endl;
	}

	for (int i = 0; i < numberOfNodes; i++) {
		for (int j = i; j < numberOfNodes; j++) {
			graph[i][j] = graph[j][i] = calcDistance(coords[i][0], coords[i][1], coords[j][0], coords[j][1]);
		}
	}

	source.close();
}

int getClosestCity(double(&graph)[MAX_SIZE][MAX_SIZE], int city) {

	double min = graph[city][0];
	int index = 0;

	while (!min) {
		min = graph[city][++index];
		if (index == numberOfNodes - 1) break;
	}

	for (int i = index; i < numberOfNodes; i++) {
		if (graph[city][i] && graph[city][i] < min) {
			min = graph[city][i];
			index = i;
		}
	}

	return index;
}

double greedy(double(&graph)[MAX_SIZE][MAX_SIZE], int start) {

	double roadsFromStart[MAX_SIZE];
	int ourCity = start, nextCity = 0, lastCity, bestCycle[MAX_SIZE + 1];
	double result = 0;

	for (int i = 0; i < numberOfNodes; i++) roadsFromStart[i] = graph[start][i];
	for (int i = 0; i < numberOfNodes; i++) {

		nextCity = getClosestCity(graph, ourCity);
		result += graph[ourCity][nextCity];
		for (int j = 0; j < numberOfNodes; j++) graph[j][ourCity] = graph[ourCity][j] = 0;
		if (i == numberOfNodes - 1) lastCity = ourCity;
		bestCycle[i] = ourCity;
		ourCity = nextCity;
	}

	cout << endl << "GREEDY BEST CYCLE: ";
	for (int i = 0; i < numberOfNodes; i++) {
		cout << bestCycle[i] << " ";
	}

	return result + roadsFromStart[lastCity];
}

double ACO(double(&graph)[MAX_SIZE][MAX_SIZE]) {

	static double pheromones[MAX_SIZE][MAX_SIZE], visibilities[MAX_SIZE][MAX_SIZE],
		poweredPheromones[MAX_SIZE][MAX_SIZE], startingPheromones[MAX_SIZE][MAX_SIZE];
	double probabilities[MAX_SIZE];
	double alpha = 1, beta = 5, ro = 0.1, delta = 0.1, q0 = 0.9, fi = 0.92, result = 1000000, iterResult, den, probabilitiesSum,
		shuffledNumber, maximalProduct, temp, cycleLengths[MAX_SIZE], qshuffle, nums[MAX_SIZE];

	bool visited[MAX_SIZE], resultChanged;
	static int paths[MAX_SIZE][MAX_SIZE + 1];
	int start, startTime = time(0), ourCity, nextCity, bestCity, j, Q = 10, RK = numberOfNodes / 10,
		bestCycle[MAX_SIZE + 1], iterBestCycle[MAX_SIZE + 1], stagnation = 0;
	mt19937 shuffle(time(0));
	ofstream zapis;

	for (int i = 0; i < numberOfNodes; i++) {
		for (j = 0; j < numberOfNodes; j++) visibilities[i][j] = pow(1 / graph[i][j], beta);
	}

	for (int i = 0; i < numberOfNodes; i++) {
		for (j = 0; j < numberOfNodes; j++) {
			startingPheromones[i][j] = startingPheromones[j][i] = 1.5 / graph[i][j] / numberOfNodes;
			pheromones[i][j] = pheromones[j][i] = startingPheromones[i][j];
		}
	}

	for (int t = 0; t < 300; t++) { // kolejna kolonia

		resultChanged = false;
		fill(cycleLengths, cycleLengths + MAX_SIZE, 0);
		iterResult = 10000000;

		for (int i = 0; i < numberOfNodes; i++) { // kolejna mrówa

			fill(visited, visited + MAX_SIZE, false);
			ourCity = start = shuffle() % numberOfNodes;

			for (int j = 0; j < numberOfNodes; j++) {
				for (int k = j + 1; k < numberOfNodes; k++) {
					poweredPheromones[j][k] = poweredPheromones[k][j] = pheromones[j][k];
				}
			}

			for (j = 0; j < numberOfNodes - 1; j++) { // kolejny wierzcholek do wybrania
				if (j != 0) ourCity = nextCity;

				visited[ourCity] = true;
				paths[i][j] = ourCity;
				probabilitiesSum = 0;
				qshuffle = double(shuffle() % 10000 + 1) / 10000;

				if (t == 0) {
					while (true) {
						shuffledNumber = shuffle() % numberOfNodes;
						if (!visited[int(shuffledNumber)]) break;
					}
					nextCity = int(shuffledNumber);
				}
				else if (qshuffle > q0) {
					den = 0;
					for (int k = 0; k < numberOfNodes; k++) { // liczenie mianownika
						if (!visited[k]) {
							nums[k] = poweredPheromones[ourCity][k] * visibilities[ourCity][k];
							den += nums[k];
						}
					}
					for (int k = 0; k < numberOfNodes; k++) { // liczenie prawdopodobieństw
						if (visited[k]) probabilities[k] = -1;
						if (!visited[k]) {
							probabilitiesSum += nums[k] / den;
							probabilities[k] = probabilitiesSum;
						}
					}
					shuffledNumber = double(shuffle() % 100000000 + 1) / 100000000;
					for (int k = 0; k < numberOfNodes; k++) {
						if (shuffledNumber <= probabilities[k]) {
							nextCity = k;
							break;
						}
					}
				}
				else {
					maximalProduct = 0;
					for (int k = 0; k < numberOfNodes; k++) {
						if (!visited[k]) {
							temp = poweredPheromones[ourCity][k] * visibilities[ourCity][k];
							if (temp > maximalProduct) {
								maximalProduct = temp;
								bestCity = k;
							}
						}
					}
					pheromones[ourCity][nextCity] = pheromones[nextCity][ourCity] = (1 - fi) * pheromones[nextCity][ourCity] + fi * startingPheromones[nextCity][ourCity];
					nextCity = bestCity;
				}
				cycleLengths[i] += graph[ourCity][nextCity];
			}
			paths[i][j++] = nextCity;
			paths[i][j] = start;
			cycleLengths[i] += graph[start][nextCity];
			if (cycleLengths[i] < result) {
				for (j = 0; j <= numberOfNodes; j++) {
					bestCycle[j] = paths[i][j];
				}
				result = cycleLengths[i];
				resultChanged = true;
			}
			if (cycleLengths[i] < iterResult) {
				for (j = 0; j <= numberOfNodes; j++) {
					iterBestCycle[j] = paths[i][j];
				}
				iterResult = cycleLengths[i];
			}

		}

		if (!resultChanged) stagnation++;
		else stagnation = 0;

		for (int i = 0; i < numberOfNodes; i++) {
			for (j = 0; j < numberOfNodes; j++) {
				pheromones[i][j] = pheromones[i][j] * (1 - ro);
			}
		}

		for (int i = 0; i < numberOfNodes - 1; i++) {
			pheromones[iterBestCycle[i]][iterBestCycle[i + 1]] = pheromones[iterBestCycle[i + 1]][iterBestCycle[i]] = pheromones[iterBestCycle[i]][iterBestCycle[i + 1]] + Q / result;
		}
		pheromones[iterBestCycle[0]][iterBestCycle[numberOfNodes - 1]] = (1 - fi) * pheromones[iterBestCycle[0]][iterBestCycle[numberOfNodes - 1]] + Q / result;

		for (int i = 0; i < numberOfNodes; i++) {
			for (j = 0; j <= numberOfNodes; j++) paths[i][j] = 0;
		}
	}

	zapis.open(to_string(filecounter++) + "wyniki" + ".txt");
	for (int i = 0; i < numberOfNodes; i++) zapis << bestCycle[i] << " ";
	zapis.close();
	cout << "TIME:" << time(0) - startTime << endl;

	return result;
}

int main() {

	srand(time(0));
	char answer;

	cout << "Czy chcesz wygenerowac instancje? (y/n)" << endl;
	cin >> answer;

	if (answer == 'y') createInstance();
	cout << "Wprowadz nazwe pliku z instancja" << endl;
	cin >> filename;
	readInstance();

	for (int i = 0; i < 1; i++) {
		cout << ACO(graph) << endl;
	}
	cout << endl << "GREEDY RESULT: " << greedy(graph, 0) << endl;

	while (1);
	return 0;
}