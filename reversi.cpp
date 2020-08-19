//Program is able to play Reversi against an user, or itself
//Program uses Monte-Carlo search, either with (Improved) or without (Pure) optimizations decide its next move
//Created by: Rizzak
#include <iostream>
#include <vector>
#include <time.h>
#include <algorithm>
#include <cstring>

using namespace std;

const int BOARD_SIZE = 8;
const int PLAYOUTS = 10000;
const int FSP_N = 5;
const double RELATIVE_SCATTER = 0.3;

int board[BOARD_SIZE][BOARD_SIZE];
int legalMoves[BOARD_SIZE][BOARD_SIZE];

int PlayerOne;
int PlayerTwo;
int nextMove[2];

//Directional Vectors
int DirX[8] = {0, 1, 1, 1, 0, -1, -1, -1};
int DirY[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

void Initialize_Board(){

    board[3][3] = board[4][4] = 1;
    board[3][4] = board[4][3] = 2;

    srand(time(NULL));
}

void Show_Board(){
    cout << endl << "1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | " << endl;
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[j][i] == 2)
                cout << "O | ";
            else if(board[j][i] == 1)
                cout << "X | ";
            else if(board[j][i] == 0 && legalMoves[j][i] == 1)
                cout << "? | ";
            else
                cout << ". | ";
        }
        cout << i+1 << endl;
    }
    cout << endl;
}

int Check_Straight(int x, int y, int k, bool update){
    int me = board[x][y];
    int opponent = 3 - me;
    int count = 0;
    bool found = false;
    int flag[BOARD_SIZE][BOARD_SIZE] = {{0}};
    int tx = x, ty = y;
    for(int i = 0; i < BOARD_SIZE; i++){
        tx += DirX[k];
        ty += DirY[k];
        if(tx < 0 || tx >= BOARD_SIZE || ty < 0 || ty >= BOARD_SIZE){
            break;
        }
        if(board[tx][ty] == opponent){
            count++;
            flag[tx][ty] = 1;
        }
        else if(board[tx][ty] == me){
            found = true;
            break;
        }
        else
            break;
    }
    if(found && count > 0 && update){ //this is only for if update==true
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                if (flag[i][j] == 1) {
                    if (board[i][j] != 0)
                        board[i][j] = 3 - board[i][j]; //flip the colors
                }
    }
    if(found && count > 0)
        return count;
    else
        return 0;
}

bool Check_Cross(int x, int y, bool update){
    if(x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE || board[x][y] == 0)
        return false;
    int opponent = 3 - board[x][y];
    int count = 0;
    int dx, dy;
    for(int k = 0; k < 8; k++)
    {
        dx = x + DirX[k];
        dy = y + DirY[k];
        if(dx < 0 || dx >= BOARD_SIZE || dy < 0 || dy >= BOARD_SIZE)
            continue;
        if(board[dx][dy] == opponent)
            count += Check_Straight(x, y, k, update);
    }
    return count > 0;
}

int Find_Legal_Moves(int player){
    int legalCount = 0;
    //wipe the legalMoves array
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            legalMoves[i][j] = 0;

    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0) { //only care about empty spaces
                if (i > 0 && i < BOARD_SIZE - 1 && j > 0 && j < BOARD_SIZE - 1) {
                    if ((board[i - 1][j - 1] == 0 || board[i - 1][j - 1] == player) &&
                        (board[i - 1][j] == 0 || board[i - 1][j] == player) &&
                        (board[i - 1][j + 1] == 0 || board[i - 1][j + 1] == player) &&
                        (board[i][j - 1] == 0 || board[i][j - 1] == player) &&
                        (board[i][j + 1] == 0 || board[i][j + 1] == player) &&
                        (board[i + 1][j - 1] == 0 || board[i + 1][j - 1] == player) &&
                        (board[i + 1][j] == 0 || board[i + 1][j] == player) &&
                        (board[i + 1][j + 1] == 0 || board[i + 1][j + 1] == player)) {
                        //the empty space is surrounded by empty space or player's own pieces
                        continue;
                    }
                }
                board[i][j] = player;
                if (Check_Cross(i, j, false)) {
                    legalMoves[i][j] = 1;
                    legalCount++;
                }
                board[i][j] = 0;
            }
        }
    return legalCount;
}

//Game_Over returns:
//-1: Not Over
//0: Tied
//1: PlayerOne
//2: PlayerTwo
int Game_Over(){
    //finding legal moves for players 1 and 2
    int lm1 = Find_Legal_Moves(1);
    int lm2 = Find_Legal_Moves(2);
    int count1 = 0, count2 = 0;
    if(lm1 == 0 && lm2 == 0){ //neither of the players have legal moves left
        //Count pieces
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                if(board[i][j] == 1)
                    count1++;
                else if(board[i][j] == 2)
                    count2++;
            }
        }

        if(count1 > count2)
            return 1; //PlayerOne victory
        else if(count1 < count2)
            return 2; //PlayerTwo victory
        else
            return 0; //Tied
    }
    return -1; //Not over
}

vector<pair<int,int> > FSP(vector<pair<int,int> > moves, int whoseTurn){
    int n = (int)moves.size();
    if(n <= FSP_N){
        return moves;
    }
    // cout << "Original Moves" << endl;
    // for(auto i:moves)
    //     cout << i.first << ":" << i.second << endl;

    vector<int>scores (n, 0);
    for(int i = 0; i < n; i++){
        int x = moves[i].first, y = moves[i].second;
        int boardCopy[BOARD_SIZE][BOARD_SIZE];
        memcpy(boardCopy, board, sizeof(board));

        //place the piece
        board[x][y] = whoseTurn;
        Check_Cross(x, y, true);

        //corners: is player's own +5, is opponent's -5
        if(board[0][0] == whoseTurn)
            scores[i] += 5;
        else if(board[0][0] == 3-whoseTurn)
            scores[i] -= 5;

        if(board[7][0] == whoseTurn)
            scores[i] += 5;
        else if(board[7][0] == 3-whoseTurn)
            scores[i] -= 5;

        if(board[0][7] == whoseTurn)
            scores[i] += 5;
        else if(board[0][7] == 3-whoseTurn)
            scores[i] -= 5;

        if(board[7][7] == whoseTurn)
            scores[i] += 5;
        else if(board[7][7] == 3-whoseTurn)
            scores[i] -= 5;

        //X-square: is player's own -2, is opponent's +2
        if(board[1][1] == whoseTurn)
            scores[i] -= 2;
        else if(board[1][1] == 3-whoseTurn)
            scores[i] += 2;

        if(board[6][1] == whoseTurn)
            scores[i] -= 2;
        else if(board[6][1] == 3-whoseTurn)
            scores[i] += 2;

        if(board[1][6] == whoseTurn)
            scores[i] -= 2;
        else if(board[1][6] == 3-whoseTurn)
            scores[i] += 2;

        if(board[6][6] == whoseTurn)
            scores[i] -= 2;
        else if(board[6][6] == 3-whoseTurn)
            scores[i] += 2;

        //C-square: is player's own -1, is opponent's +1
        if(board[0][1] == whoseTurn)
            scores[i] -= 1;
        else if(board[0][1] == 3-whoseTurn)
            scores[i] += 1;

        if(board[1][0] == whoseTurn)
            scores[i] -= 1;
        else if(board[1][0] == 3-whoseTurn)
            scores[i] += 1;

        if(board[0][6] == whoseTurn)
            scores[i] -= 1;
        else if(board[0][6] == 3-whoseTurn)
            scores[i] += 1;

        if(board[6][0] == whoseTurn)
            scores[i] -= 1;
        else if(board[6][0] == 3-whoseTurn)
            scores[i] += 1;

        if(board[1][7] == whoseTurn)
            scores[i] -= 1;
        else if(board[1][7] == 3-whoseTurn)
            scores[i] += 1;

        if(board[7][1] == whoseTurn)
            scores[i] -= 1;
        else if(board[7][1] == 3-whoseTurn)
            scores[i] += 1;

        if(board[6][7] == whoseTurn)
            scores[i] -= 1;
        else if(board[6][7] == 3-whoseTurn)
            scores[i] += 1;

        if(board[7][6] == whoseTurn)
            scores[i] -= 1;
        else if(board[7][6] == 3-whoseTurn)
            scores[i] += 1;

        //sweet 16: is player's own +2, is opponent's -2
        for(int a = 2; a <= 5; a++)
        {
            for(int b = 2; b <= 5; b++)
            {
                if(board[a][b] == whoseTurn)
                    scores[i] += 2;
                else if(board[a][b] == 3-whoseTurn)
                    scores[i] -= 2;
            }
        }

        //others: is player's own +1, is opponent's -1
        for(int a = 0; a <= 1; a++)
        {
            for(int b = 2; b <= 5; b++)
            {
                if(board[a][b] == whoseTurn)
                    scores[i] += 1;
                else if(board[a][b] == 3-whoseTurn)
                    scores[i] -= 1;
            }
        }

        for(int a = 6; a <= 7; a++)
        {
            for(int b = 2; b <= 5; b++)
            {
                if(board[a][b] == whoseTurn)
                    scores[i] += 1;
                else if(board[a][b] == 3-whoseTurn)
                    scores[i] -= 1;
            }
        }

        for(int a = 2; a <= 5; a++)
        {
            for(int b = 0; b <= 1; b++)
            {
                if(board[a][b] == whoseTurn)
                    scores[i] += 1;
                else if(board[a][b] == 3-whoseTurn)
                    scores[i] -= 1;
            }
        }

        for(int a = 2; a <= 5; a++)
        {
            for(int b = 6; b <= 7; b++)
            {
                if(board[a][b] == whoseTurn)
                    scores[i] += 1;
                else if(board[a][b] == 3-whoseTurn)
                    scores[i] -= 1;
            }
        }

        // Show_Board();
        // cout << "Score: " << scores[i];

        //restore board
        memcpy(board, boardCopy, sizeof(boardCopy));
    }

    for(int j = 0; j < n - FSP_N; j++)
    {
        int i = min_element(scores.begin(), scores.end()) - scores.begin();
        moves.erase(moves.begin() + i);
        scores.erase(scores.begin() + i);
    }

    // cout << "Trimmed Moves" << endl;
    // for(auto i:moves)
    //     cout << i.first << ":" << i.second << endl;

    return moves;
}
void printVectorInt(vector<int> vec, string name)
{
    cout << "\nVector " << name << ",size = " << vec.size() << ": ";
    for (int i = 0; i < vec.size(); i++)
        cout << vec[i] << " ";
}

//simple heuristic - Q = num of chips of player whose turn
int findQ(int whoseTurn, int (&board)[BOARD_SIZE][BOARD_SIZE])
{
    int counter = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == whoseTurn) { //is a legal move
                counter++;
            }
        }
    }
    return counter;
}

int Random_Playout(int whoseTurn, bool improved)
{
    int result = Game_Over();
    if(result != -1)
        return result;
    if(Find_Legal_Moves(whoseTurn) == 0) //if there's no legal moves available here
        return Random_Playout(3-whoseTurn, improved);
    vector<pair<int,int> > availableMoves;

    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(legalMoves[i][j] == 1){ //is a legal move
                availableMoves.push_back(pair<int,int>(i,j));
            }
        }
    }

    if(improved){
        availableMoves = FSP(availableMoves, whoseTurn);

        vector<int> qVec; // our qualities for every possible move
        int tmpBoard[BOARD_SIZE][BOARD_SIZE];
        for (int i = 0; i < availableMoves.size(); i++)
        {
            memcpy(tmpBoard, board, sizeof(board));
            int x = availableMoves[i].first;
            int y = availableMoves[i].second;
            board[x][y] = whoseTurn;
            Check_Cross(x, y, true);
            qVec.push_back(findQ(whoseTurn, board));
            memcpy(board, tmpBoard, sizeof(board));
        }
        double rk = (rand() % 10000) / 10000.; //random component
        float maxF = qVec[0] * (1 + 2 * RELATIVE_SCATTER * (rk - 0.5)), newF = 0.;
        int nextMove = 0;
        for (int i = 1; i < qVec.size(); i++)
        {
            rk = (rand() % 10000) / 10000.;
            newF = qVec[i] * (1 + 2 * RELATIVE_SCATTER * (rk - 0.5));
            if (newF > maxF)
            {
                maxF = newF;
                nextMove = i;
            }
        }
        int x = availableMoves[nextMove].first;
        int y = availableMoves[nextMove].second;
        board[x][y] = whoseTurn;
        Check_Cross(x, y, true);
        return Random_Playout(3 - whoseTurn, improved);
    }

    int nextMove = rand() % (int) availableMoves.size();
    int x = availableMoves[nextMove].first;
    int y = availableMoves[nextMove].second;
    board[x][y] = whoseTurn;
    Check_Cross(x, y, true);
    return Random_Playout(3-whoseTurn, improved);
}

void Human_Move(int whoseTurn){
    int n = 0;
    for(int i = 0; i < BOARD_SIZE; i++)
        for(int j = 0; j < BOARD_SIZE; j++)
            if(legalMoves[i][j] == 1) //is a legal move
                n++;
    if(n == 0) //no moves available
    {
        cout << "Human has no moves available! Giving up turn..." << endl;
        return;
    }

    int col = -1;
    int row = -1;
    while(1){
        cout << "Plase choose the column of your next move(1-8)" << endl;
        cin >> col;
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Plase choose the row of your next move(1-8)" << endl;
        cin >> row;
        cin.clear();
        cin.ignore(1000, '\n');
        if(col < 1 || col > 8 || row < 1 || row > 8)
        {
            cout << "Error: Invalid Position! Try again" << endl;
            continue;
        }
        col--; //indexing
        row--;
        if(legalMoves[col][row] != 1)
        {
            cout << "That's not a legal move! Try again" << endl;
            continue;
        }
        break;
    }
    board[col][row] = whoseTurn;
    Check_Cross(col, row, true);
    cout << "Human makes a move at column " << col+1 << " and row " << row+1 << endl;
    return;
}

void Monte_Move(int whoseTurn){



    vector<pair<int,int> > availableMoves;
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(legalMoves[i][j] == 1){ //is a legal move
                availableMoves.push_back(pair<int,int>(i,j));
            }
        }
    }
    int n = (int) availableMoves.size();
    if(n == 0) //no moves available
    {
        cout << "Pure Monte has no moves available! Giving up turn..." << endl;
        return;
    }

    vector<int> scores(n, 0);
    int boardCopy[BOARD_SIZE][BOARD_SIZE];
    memcpy(boardCopy, board, sizeof(board)); //deep copy
    for(int i = 0; i < n; i++)
    {
        int x = availableMoves[i].first;
        int y = availableMoves[i].second;
        for(int p = 0; p < PLAYOUTS; p++)
        {

            board[x][y] = whoseTurn;
            int winner = Random_Playout(3-whoseTurn, false);
            // Show_Board();
            if(winner == whoseTurn) //monte wins
                scores[i]++;
            else if(winner == 3-whoseTurn) //monte lost
                scores[i]--;
            memcpy(board, boardCopy, sizeof(board)); //restore board
        }
    }
    int i = max_element(scores.begin(), scores.end()) - scores.begin();
    int x = availableMoves[i].first;
    int y = availableMoves[i].second;
    int monteTime=0;

    board[x][y] = whoseTurn;
    Check_Cross(x, y, true);
}

void Improved_Move(int whoseTurn){

    vector<pair<int,int> > availableMoves;
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(legalMoves[i][j] == 1){ //is a legal move
                availableMoves.push_back(pair<int,int>(i,j));
            }
        }
    }

    availableMoves = FSP(availableMoves, whoseTurn);
    int n = (int) availableMoves.size();
    if(n == 0) //no moves available
    {
        cout << "Improved Monte has no moves available! Giving up turn..." << endl;
        return;
    }

    vector<int> scores(n, 0);
    int boardCopy[BOARD_SIZE][BOARD_SIZE];
    memcpy(boardCopy, board, sizeof(board)); //deep copy
    for(int i = 0; i < n; i++)
    {
        int x = availableMoves[i].first;
        int y = availableMoves[i].second;
        for(int p = 0; p < PLAYOUTS; p++)
        {
//            if(float(clock()-startTime) / CLOCKS_PER_SEC > 5) //longer than 5 sec to think
//            {
//                cout << "Improved Monte took too long to think (> 5 seconds), program terminating";
//                exit(EXIT_FAILURE);
//            }
            board[x][y] = whoseTurn;
            int winner = Random_Playout(3-whoseTurn, true);
            // Show_Board();
            if(winner == whoseTurn) //monte wins
                scores[i]++;
            else if(winner == 3-whoseTurn) //monte lost
                scores[i]--;
            memcpy(board, boardCopy, sizeof(board)); //restore board
        }
    }
    int i = max_element(scores.begin(), scores.end()) - scores.begin();
    int x = availableMoves[i].first;
    int y = availableMoves[i].second;
//    cout << "Improved Monte makes move to column " << x + 1 << " row " << y + 1 << endl;
//    cout << "Improved Monte took " << float(clock()-startTime) / CLOCKS_PER_SEC
//         << " seconds to think" << endl;
    board[x][y] = whoseTurn;
    Check_Cross(x, y, true);
}

int main() {
    int rounds = 0;
    int result = 0;
    int turn = 1;
    cout << "Welcome to Reversi! Please choose the players playing today" << endl;
    cout << "Note that the potential legal moves will be labelled with a ? for each player" << endl;
    // cout << "Player 1 [X]: Enter 0 for Manual, 1 for Computer Using Pure Monte Carlo Search, 2 for Computer using Improved Monte Search, 3 for IMS with RW" << endl;
    //cin >> PlayerOne;
    //cout << "Player 2 [O]: Enter 0 for Manual, 1 for Computer Using Pure Monte Carlo Search, 2 for Computer using Improved Monte Search, 3 for IMS with RW" << endl;
    //cin >> PlayerTwo;
    PlayerOne = 1;
    PlayerTwo = 2;
    if (PlayerOne < 0 || PlayerOne > 3 || PlayerTwo < 0 || PlayerTwo > 3 || (PlayerOne == 0 && PlayerTwo == 0))
    {
        cout << "Error: Invalid Player Selections, Program Terminating" << endl;
        return -1;
    }
    int numOfGames = 30;

    int freshBoard[BOARD_SIZE][BOARD_SIZE] = { {0} };
    Initialize_Board();
    memcpy(freshBoard, board, sizeof(board));
    int freshLegalMoves[BOARD_SIZE][BOARD_SIZE] = { {0} };
    int monteWins = 0, ImprovedWins = 0,  Ties = 0;
    bool isGameEnded = false;

    for (int i = 0; i < numOfGames; i++)
    {
//        clock_t startTime = clock();
        //init section
        isGameEnded = false;
        if (i % 4 == 0)
        {
            PlayerOne = 3;
            PlayerTwo = 2;
        }
        else if (i % 4 == 1)
        {
            PlayerOne = 4;
            PlayerTwo = 2;
        }
        else if (i % 4 == 2)
        {
            PlayerOne = 2;
            PlayerTwo = 5;
        }
        else
        {
            PlayerOne = 2;
            PlayerTwo = 6;
        }
        memcpy(board, freshBoard, sizeof(board));
        memcpy(legalMoves, freshLegalMoves, sizeof(board));
        //one game session
        while (true) //upper round limit
        {
            Find_Legal_Moves(turn);
//            Show_Board();
            if (turn == 1) //PlayerOne
            {
                switch (PlayerOne) {
                    case 0:
                        Human_Move(turn);
                        break;
                    case 1:
                        Monte_Move(turn);

                        break;
                    case 2:
                        Improved_Move(turn);
                        break;

                }
                turn = 2;
            }
            else { //PlayerTwo
                switch (PlayerTwo) {
                    case 0:
                        Human_Move(turn);
                        break;
                    case 1:
                        Monte_Move(turn);
                        break;
                    case 2:
                        Improved_Move(turn);
                        break;

                }
                turn = 1;
            }
            result = Game_Over();
            if (result != -1) {
//                Show_Board();
                switch (result) {
                    case 0:
//                        cout << "The Game is Tied!" << endl;
                        isGameEnded = true;
                        Ties++;

                        break;
                    case 1:
                        switch (PlayerOne) {
                            case 0:
//                                cout << "Human has Won!" << endl;
                                isGameEnded = true;
                                break;
                            case 1:
//                                cout << "Pure Monte has Won!" << endl;
                                isGameEnded = true;
                                monteWins++;

                                break;
                            case 2:
//                                cout << "Improved Monte has Won!" << endl;
                                isGameEnded = true;
                                ImprovedWins++;

                                break;

                        }
                        break;
                    case 2:
                        switch (PlayerTwo) {
                            case 0:
                                cout << "Human has Won!" << endl;
                                isGameEnded = true;
                                break;
                            case 1:
//                                cout << "Pure Monte has Won!" << endl;
                                isGameEnded = true;
                                monteWins++;

                                break;
                            case 2:
//                                cout << "Improved Monte has Won!" << endl;
                                isGameEnded = true;
                                ImprovedWins++;

                                break;

                        }
                        break;
                }
            }
            if (isGameEnded)
                break;
        }
//        cout << "Total game time " << float(clock()-startTime) / CLOCKS_PER_SEC<< " seconds" << endl;
    }
//    cout << "Results of simultation: \nTies: " << Ties << "\nMonte: " << monteWins << "\nImproved: " << ImprovedWins << "\nRW with all FSP: " << RW00Wins
//         <<"\nRW with outer FSP: " << RW02Wins << "\nRW with inner FSP: " << RW04Wins << "\nRW w/o FSP: "<<RW06Wins;
    cout<< monteWins << " Monte winning numbers of games" << endl;;
    cout<< ImprovedWins<< " Improved winning numbers of games"<< endl;;
    cout<< Ties << " Ties numbers of games"<< endl;

    return 0;
}