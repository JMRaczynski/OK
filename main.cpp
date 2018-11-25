#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#define MAX_SIZE 1500

using namespace std;

int numberOfNodes;
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
        destination << i + 1 << " " << rand() % 10 + 1 << " " << rand() % 10 + 1 << endl;
    }

    destination.close();
}

void readInstance() {

    int buffer;
    ifstream source;
    source.open(filename + ".txt");
    source >> numberOfNodes;

    int coords[numberOfNodes][2];
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

    /*for (int i = 0; i < numberOfNodes; i++) {
        for (int j = 0; j < numberOfNodes; j++) cout << graph[i][j] << "\t";
        cout << endl;
    }*/

    source.close();
}

int getClosestCity(double (&graph)[MAX_SIZE][MAX_SIZE], int city) {

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

double greedy(double (&graph)[MAX_SIZE][MAX_SIZE], int start) {

    double roadsFromStart[numberOfNodes];
    int ourCity = start, nextCity = 0, lastCity, bestCycle[MAX_SIZE + 1];
    double result = 0;

    for (int i = 0; i < numberOfNodes; i++) roadsFromStart[i] = graph[start][i];
    for (int i = 0; i < numberOfNodes; i++) {

        /*cout << i << ": " << result << endl;
        for (int j = 0; j < graph.size(); j++) {
        for (int k = 0; k < graph.size(); k++) {			//DO DEBUGOWANIA
        cout << graph[j][k] << " ";
        }
        cout << endl;
        }*/

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

double ACO(double (&graph)[MAX_SIZE][MAX_SIZE]) {

    static double pheromones[MAX_SIZE][MAX_SIZE] = {0};
    double probabilities[MAX_SIZE];
    double alpha = 1, beta = 8, ro = 0.6, q0 = 0.22, result = 100000000, num, den, probabilitiesSum, shuffledNumber,
    maximalProduct, temp, cycleLengths[MAX_SIZE], qshuffle;
    bool visited[MAX_SIZE];
    static int paths[MAX_SIZE][MAX_SIZE + 1];
    int start, ourCity, nextCity, bestCity, j, Q = 1000, bestCycle[MAX_SIZE + 1];
    mt19937 shuffle(time(0));

    /*for (int i = 0; i < numberOfNodes; i++) {
        //for (j = 0; j < numberOfNodes; j++) cout << graph[i][j] << " ";
        cout << min(*min_element(graph[i], graph[i] + i), *min_element(graph[i] + i + 1, graph[i] + numberOfNodes)) << " ";
        cout << endl;
    }*/

    for (int t = 0; t < 1000; t++) { // kolejna kolonia
        /*cout << endl << endl << endl << "COLONY NUMBER: " << t << endl;
        for (int i = 0; i < numberOfNodes; i++) {
            for (j = 0; j < numberOfNodes; j++) cout << pheromones[i][j] << "\t";
            cout << endl;
        }*/
        fill(cycleLengths, cycleLengths + MAX_SIZE, 0);
        for (int i = 0; i < numberOfNodes; i++) { // kolejna mrówa

            //cout << endl << "ANT NUMBER: " << i << endl;

            fill(visited, visited + MAX_SIZE, false);
            ourCity = start = shuffle() % numberOfNodes;

            for (j = 0; j < numberOfNodes - 1; j++) { // kolejny wierzcholek do wybrania
                if (j != 0) ourCity = nextCity;
                visited[ourCity] = true;

                //cout << "VISITED: ";
                //for (int k = 0; k < numberOfNodes; k++) cout << visited[k] << ", ";
                //cout << endl;

                paths[i][j] = ourCity;
                probabilitiesSum = 0;
                qshuffle = double(shuffle() % 10000 + 1) / 10000;

                //cout << qshuffle << endl;

                if (qshuffle < q0 || t == 0) {
                    for (int k = 0; k < numberOfNodes; k++) { // liczenie prawdopodobieństw
                        if (visited[k]) probabilities[k] = -1;
                        else {
                            den = 0;
                            num = pow(pheromones[ourCity][k], alpha) / pow(graph[ourCity][k], beta);

                            //cout << "NUM: " << num << endl;

                            for (int l = 0; l < numberOfNodes; l++) { // liczenie mianownika
                                if (!visited[l]) {
                                    den += pow(pheromones[ourCity][l], alpha) / pow(graph[ourCity][l], beta);
                                }
                            }

                            //cout << "DEN: " << den << endl;
                        }
                        if (den != 0 && !visited[k]) {
                            probabilitiesSum += num / den;
                            probabilities[k] = probabilitiesSum;
                        }
                    }
                    if (probabilitiesSum != 0 && den != 0) {
                        shuffledNumber = double(shuffle() % 10000 + 1) / 10000;
                        for (int k = 0; k < numberOfNodes; k++) {
                            if (!visited[k] && shuffledNumber < probabilities[k]) {
                                nextCity = k;
                                break;
                            }
                        }

                        /*cout << "PROBABILITES: ";
                        for (int k = 0; k < numberOfNodes; k++) {
                            cout << probabilities[k] << ", ";
                        }
                        cout << endl;*/
                    }
                    else {
                        while (true) {
                            shuffledNumber = shuffle() % numberOfNodes;
                            if (!visited[int(shuffledNumber)]) break;
                        }
                        nextCity = int(shuffledNumber);
                    }
                }
                else {
                    maximalProduct = 0;
                    for (int k = 0; k < numberOfNodes; k++) {
                        if (!visited[k]) {
                            temp = pow(pheromones[ourCity][k], alpha) / pow(graph[ourCity][k], beta);
                            if (temp > maximalProduct) {
                                maximalProduct = temp;
                                bestCity = k;
                            }
                        }
                    }
                    nextCity = bestCity;
                }
                cycleLengths[i] += graph[ourCity][nextCity];
            }
            paths[i][j++] = nextCity;
            paths[i][j] = start;
            cycleLengths[i] += graph[start][nextCity];
            if (cycleLengths[i] < result) {
                for(j = 0; j <= numberOfNodes; j++) {
                    bestCycle[j] = paths[i][j];
                }
                result = cycleLengths[i];
            }

            /*cout << "PATH: ";
            for(j = 0; j <= numberOfNodes; j++) {
                cout << paths[i][j] << ", ";
            }
            cout << endl;*/

        }

        for (int i = 0; i < numberOfNodes; i++) {
            for (j = 0; j < numberOfNodes; j++) {
                pheromones[i][j] *= ro;
            }
        }
        for (int i = 0; i < numberOfNodes; i++) {
            for (j = 0; j < numberOfNodes; j++) {
                pheromones[paths[i][j]][paths[i][j + 1]] = pheromones[paths[i][j + 1]][paths[i][j]] += Q / cycleLengths[i];
            }
        }

        for (int i = 0; i < numberOfNodes; i++) {
            for (j = 0; j <= numberOfNodes; j++) paths[i][j] = 0;
        }
    }
    cout << "ACO BEST CYCLE: ";
    for (int i = 0; i <= numberOfNodes; i++) {
        cout << bestCycle[i] << " ";
    }
    cout << endl;
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

    double results[numberOfNodes];

    for (int i = 0; i < 10; i++) cout << "FINAL RESULT:" << ACO(graph) << endl;
    cout << endl << "GREEDY RESULT: " << greedy(graph, 0) << endl;

    /*for (int i = 0; i < numberOfNodes; i++) {
        readInstance();
        results[i] = greedy(graph, i);
        //cout << endl << results[i];
        //cout << i << endl;
    }

    cout << endl << "FINAL RESULT:" << *min_element(results, results + numberOfNodes);*/

    return 0;
}

