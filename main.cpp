#include <bits/stdc++.h>
using namespace std;

// Scale up
const int maxCustomer = 10;
const int maxDistributor = 3;
const int maxProvider = 2;
const int maxType = 200;

int numberOfProvider = 2;
int numberOfDistributor = 3;
int numberOfCustomer = 10;

// Cost per ton
float costPD[maxProvider][maxDistributor];
float costDD[maxDistributor][maxDistributor];
float costDC[maxDistributor][maxCustomer];

float timePD[maxProvider][maxDistributor];
float timeDD[maxDistributor][maxDistributor];
float timeDC[maxDistributor][maxCustomer];

// Storage
float sum = 0.0; // Total Cost
float store[maxDistributor][maxType];

void construct()
{
    for (int i = 0; i < maxProvider; i++)
        for (int j = 0; j < maxDistributor; j++)
            timePD[i][j] = costPD[i][j] = -1.0;

    for (int i = 0; i < maxDistributor; i++)
        for (int j = 0; j < maxDistributor; j++)
            timePD[i][j] = costDD[i][j] = -1.0;

    for (int i = 0; i < maxDistributor; i++)
        for (int j = 0; j < maxCustomer; j++)
            timePD[i][j] = costDC[i][j] = -1.0;
}

// Variables for pathGenerator
bool visited[maxDistributor];
int path[maxDistributor + 1], currentCustomer;
float penalty;

vector<vector<int>> pathSaver;

// Reset the Generator after printing path
void resetGenerator(int distributor)
{
    memset(visited, false, sizeof(visited));
    for (int node = 0; node < numberOfDistributor + 1; node++)
        path[node] = -1;
    path[0] = distributor;
}

// Save the path that generated to a vector<vector<int>>
void updatePath()
{
    stack<int> reversePath;
    for (int node = 0; node < numberOfDistributor; node++)
        if (path[node] != -1)
            reversePath.push(path[node]);

    vector<int> currentPath;

    // Reverse the path for easier 'future'
    while (!reversePath.empty())
    {
        currentPath.push_back(reversePath.top());
        reversePath.pop();
    }

    pathSaver.push_back(currentPath);
}

// Main code of the Generator
void pathGenerator(int count, int currentDistributor)
{
    visited[currentDistributor] = true;
    for (int nextDistributor = 0; nextDistributor < numberOfDistributor; nextDistributor++)
    {
        // If this Distributor is not selected
        if (!visited[nextDistributor] && costDD[nextDistributor][currentDistributor] > 0)
        {
            // Select the Distributor
            visited[nextDistributor] = true;
            path[count] = nextDistributor;
            if (count < numberOfDistributor)
            {
                // Continue the Generator and update the current path
                pathGenerator(count + 1, nextDistributor);
                updatePath();
            }
            // Deselect the Distributor
            path[count] = -1;
            visited[nextDistributor] = false;
        }
    }
}

// Find the best Provider for each Distributor
int bestProvider(int distributor, float currPenalty, int weight)
{
    float minFee = 1e6;
    int providerIndex;

    for (int provider = 0; provider < numberOfProvider; provider++)
        if (costPD[provider][distributor] * weight + (currPenalty + timePD[provider][distributor] > 7) * (trunc(currPenalty + timePD[provider][distributor] > 7) - 7) * penalty < minFee && costPD[provider][distributor] > 0.0)
        {
            providerIndex = provider;
            minFee = costPD[provider][distributor];
        }
    return providerIndex;
}

// Find the best PD for PDC
pair<int, int> bestPD(int customer, int weight)
{
    float tmpPDC = 1e6;
    int tmpProvider, tmpDistributor;

    for (int distributor = 0; distributor < numberOfDistributor; distributor++)
        for (int provider = 0; provider < numberOfProvider; provider++)
            if (costPD[provider][distributor] > 0.0 && costDC[distributor][customer] > 0.0)
            {
                bool bigger7days = timePD[provider][distributor] + timeDC[distributor][customer] > 7;
                int penaltyDay = trunc(timePD[provider][distributor] + timeDC[distributor][customer]) - 7;

                if ((costPD[provider][distributor] + costDC[distributor][customer]) * weight + bigger7days * penaltyDay * penalty < tmpPDC)
                {
                    tmpPDC = (costPD[provider][distributor] + costDC[distributor][customer]) * weight + bigger7days * penaltyDay * penalty;
                    tmpProvider = provider;
                    tmpDistributor = distributor;
                }
            }
    return make_pair(tmpProvider, tmpDistributor);
}

// Main code
void solve(int customer, int type, float weight)
{
    // Count for directly cost
    float directly;
    pathSaver.resize(0); // Reset the path for each customer

    // Find the mininum Distributor that can supply
    float minDC = 1e6;
    int firstDC;

    for (int distributor = 0; distributor < numberOfDistributor; distributor++)
    {
        bool bigger7days = timeDC[distributor][customer] > 7.0;
        int penaltyDay = trunc(timeDC[distributor][customer] - 7);
        if (costDC[distributor][customer] > 0.0 && costDC[distributor][customer] * weight + bigger7days * penalty * penaltyDay < minDC)
        {
            firstDC = distributor;
            minDC = costDC[distributor][customer] * weight + bigger7days * penalty * penaltyDay;
        }
    }

    directly = minDC;

    // Enough?
    if (store[firstDC][type] >= weight)
    {
        sum = sum + directly; // Add the cost to the total cost

        // cout << "Before: " << store[firstDC][type] << " - After: " << store[firstDC][type] - weight << "\n";
        store[firstDC][type] -= weight;

        // Flag DC -> Sell directly
        cout << "D" << firstDC + 1 << "->C" << customer + 1 << " " << costDC[firstDC][customer] * weight << " " << directly - costDC[firstDC][customer] * weight << " " << directly << "\n";
    }
    else
    {
        float need = weight - store[firstDC][type];
        store[firstDC][type] = 0;
        // Generate the path to "owe" product
        for (int distributor = 0; distributor < numberOfDistributor; distributor++)
            if (costDC[distributor][customer] > 0.0)
            {
                resetGenerator(distributor);
                pathGenerator(1, distributor);
            }

        float minPDDC = 1e6, minTimeFee;
        int minPath; // For the first attempt

        for (int ith = 0; ith < (int)pathSaver.size(); ith++)
        {
            float tmpPDDC = 0.0; // Create a temporary cost to compare
            float timePDDC = 0.0;
            int prevNode = pathSaver[ith][0];

            for (int node = 1; node < (int)pathSaver[ith].size(); node++) // Calculate the cost
            {
                tmpPDDC = tmpPDDC + costDD[prevNode][pathSaver[ith][node]];
                timePDDC = timePDDC + timeDD[prevNode][pathSaver[ith][node]];
                prevNode = pathSaver[ith][node];
            }

            timePDDC = timePDDC + timeDC[prevNode][customer];

            float penaltyFee = (timePDDC > 7) * (trunc(timePDDC) - 7) * penalty;

            tmpPDDC = tmpPDDC + costDC[prevNode][customer] + costPD[bestProvider(pathSaver[ith][0], timePDDC, weight)][pathSaver[ith][0]];
            tmpPDDC = tmpPDDC * need + penaltyFee;

            if (tmpPDDC < minPDDC) // For the rest
            {
                minPDDC = tmpPDDC;
                minTimeFee = penaltyFee;
                minPath = ith;
            }
        }

        // Find the best PDC
        pair<int, int> bestPDC = bestPD(customer, weight);
        float tmp1st = costPD[bestPDC.first][bestPDC.second] + costDC[bestPDC.second][customer];
        float tmp2nd = timePD[bestPDC.first][bestPDC.second] + timeDC[bestPDC.second][customer];
        float minPDC = tmp1st * need + (tmp2nd > 7) * trunc(tmp2nd - 7) * penalty;

        // cout << minPDDC << " " << minPDC << "\n";
        if (minPDDC < minPDC) // Checking for the optimizer way
        {
            for (int node = 0; node < (int)pathSaver[minPath].size(); node++)
            {
                cout << "D" << pathSaver[minPath][node] + 1;
                if (node != pathSaver[minPath].size() - 1)
                    cout << "->";
            }
            cout << "->C" << customer + 1
                 << " " << minPDDC - minTimeFee
                 << " " << minTimeFee
                 << " " << minPDDC << endl;

            sum = sum + minPDDC;
        }
        else
        {
            cout << "P" << bestPDC.first + 1 << "->D" << bestPDC.second + 1 << "->C" << customer + 1
                 << " " << tmp1st * need
                 << " " << abs((tmp2nd > 7) * trunc(tmp2nd - 7) * penalty)
                 << " " << minPDC << endl;

            sum = sum + minPDC;
        }
    }
}

void input()
{
    cin >> numberOfProvider >> numberOfDistributor >> numberOfCustomer;
    construct();
    int operationSize = 3; // Input
    for (int operation = 0; operation < operationSize; operation++)
    {
        char op;
        int num;
        cin >> op >> num;
        if (op == 'P') // Cost from Provider to Distributor
            for (int i = 0; i < num; i++)
            {
                int provider, distributor;
                float Cost, Time;
                cin >> provider >> distributor >> Cost >> Time;
                costPD[provider - 1][distributor - 1] = Cost;
                timePD[provider - 1][distributor - 1] = Time;
            }
        else if (op == 'D') // Cost between Distributors
            for (int i = 0; i < num; i++)
            {
                int distributor1, distributor2;
                float Cost, Time;
                cin >> distributor1 >> distributor2 >> Cost >> Time;
                costDD[distributor1 - 1][distributor2 - 1] = Cost;
                timeDD[distributor1 - 1][distributor2 - 1] = Time;
            }
        else if (op == 'C') // Cost from Distributor to Customer
            for (int i = 0; i < num; i++)
            {
                int distributor, customer;
                float Cost, Time;
                cin >> distributor >> customer >> Cost >> Time;
                costDC[distributor - 1][customer - 1] = Cost;
                timeDC[distributor - 1][customer - 1] = Time;
            }
    }

    int numberOfType, numberOfDay, numberOfLine;
    cin >> numberOfLine;
    for (int line = 0; line < numberOfLine; line++)
    {
        int type, distributor;
        float value;
        cin >> type >> distributor >> value;
        store[distributor - 1][type - 1] = value;
    }
    cin >> penalty;

    cin >> numberOfDay;
    cout << numberOfDay << "\n";
    for (int day = 0; day < numberOfDay; day++)
    {
        int d, m, y;
        cin >> d >> m >> y >> numberOfLine;
        cout << d << "/" << m << "/" << y << " " << numberOfLine << "\n";
        for (int line = 0; line < numberOfLine; line++)
        {
            int destination, type;
            float weight;
            cin >> destination >> type >> weight;
            if (weight == 0)
                continue;
            // if (destination > 10 || weight < 0 || type > 100)
            //     continue;
            cout << line + 1 << " ";
            // cout << "\nNeed: " << weight << " Customer: " << destination << " Type: " << type << endl;
            solve(destination - 1, type - 1, weight);
        }
        cout << "\n";
    }

    cout << sum << "\n";
}

int main()
{
    cin.tie(0)->sync_with_stdio(0);
    freopen("inp.txt", "r", stdin);
    freopen("out.txt", "w", stdout);
    input();
}
