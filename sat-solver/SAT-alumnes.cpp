#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<vector<int>> occurListPositiu(301); //tamany 
vector<vector<int>> occurListNegatiu(301); 
vector<pair<int,int>>cont(301); //primer contador de las clausulas y segundo contador de conflictos
int cont_conflictes = 0;
vector<int> model;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;


void readClauses( ){
  // Skip comments
  char c = cin.get();
  while (c == 'c') {
    while (c != '\n') c = cin.get();
    c = cin.get();
  }  
  // Read "cnf numVars numClauses"
  string aux;
  cin >> aux >> numVars >> numClauses;
  clauses.resize(numClauses);  
  // Read clauses
  for (uint i = 0; i < numClauses; ++i) {
    int lit;
    while (cin >> lit and lit != 0) {
      clauses[i].push_back(lit);
      
      if(lit > 0) {
        cont[lit].first += 1;
        occurListPositiu[lit].push_back(i); // guardem l'index de la clausula on apareix el literal
      }  
      else {
        cont[-lit].first += 1;
        occurListNegatiu[-lit].push_back(i);
      }
    }
  }    
}


int currentValueInModel(int lit){
  if (lit >= 0) return model[lit];
  else {
    if (model[-lit] == UNDEF) return UNDEF;
    else return 1 - model[-lit];
  }
}


void setLiteralToTrue(int lit){
  modelStack.push_back(lit);
  if (lit > 0) model[lit] = TRUE;
  else model[-lit] = FALSE;		
}


bool propagateGivesConflict ( ) {
  while ( indexOfNextLitToPropagate < modelStack.size() ) {
    int lit = modelStack[indexOfNextLitToPropagate];
    ++indexOfNextLitToPropagate;
  
    if (lit > 0) {
      for (uint i = 0; i < occurListNegatiu[lit].size(); ++i) {
        bool someLitTrue = false;
        int numUndefs = 0;
        int lastLitUndef = 0;
        int index = occurListNegatiu[lit][i];
        for (int k=0; not someLitTrue and k < clauses[index].size(); ++k) {
	        int val = currentValueInModel(clauses[index][k]);
	        if (val == TRUE) someLitTrue = true;
	        else if (val == UNDEF){ ++numUndefs; lastLitUndef = clauses[index][k]; } //mirar tots els literals de la clausula
        }
      if (not someLitTrue and numUndefs == 0) {
        cont[lit].second += 1;
        ++cont_conflictes;
        return true; // conflict! all lits false
      }
      else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);	
      }    
    }
    else {
      for (uint i = 0; i < occurListPositiu[-lit].size(); ++i) {
        bool someLitTrue = false;
        int numUndefs = 0;
        int lastLitUndef = 0;
        int index = occurListPositiu[-lit][i];
        for (int k=0; not someLitTrue and k < clauses[index].size(); ++k) {
	        int val = currentValueInModel(clauses[index][k]);
	        if (val == TRUE) someLitTrue = true;
	        else if (val == UNDEF){ ++numUndefs; lastLitUndef = clauses[index][k]; }
        }
      if (not someLitTrue and numUndefs == 0) {
        cont[-lit].second += 1;
        ++cont_conflictes;
        return true; // conflict! all lits false
      }
      else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);	
      }
    }
  }
  return false;
}


void backtrack(){
  uint i = modelStack.size() -1;
  int lit = 0;
  while (modelStack[i] != 0){ // 0 is the DL mark
    lit = modelStack[i];
    model[abs(lit)] = UNDEF;
    modelStack.pop_back();
    --i;
  }
  // at this point, lit is the last decision
  modelStack.pop_back(); // remove the DL mark
  --decisionLevel;
  indexOfNextLitToPropagate = modelStack.size();
  setLiteralToTrue(-lit);  // reverse last decision
}

// Heuristic for finding the next decision literal:
int getNextDecisionLiteral(){
  int max_cont = -1; //guarda la variable indefinida que apareix en mes clausules
  int nextLit; //guardem els possibles literals seguents
  //int max_undef = -1;
  
  for (uint i = 1; i < cont.size(); ++i) {
    if ((cont[i].first*0.25 + cont[i].second*0.75) > max_cont and model[i] == UNDEF) {
      max_cont = (cont[i].first*0.25 + cont[i].second*0.75);
      nextLit = i; //guardem el possible nextlit
    }
  } 
  /*
  else {
    for (uint i = 1; i < cont.size(); ++i) {
      if (cont[i].second > max_undef and model[i] == UNDEF) {
        max_undef = cont[i].first;
        nextLit = i; //guardem el possible nextlit
      }
    } 
  }*/
  if (max_cont != -1) return nextLit;
  return 0; //tots els literals son definits
}




void checkmodel(){
  for (uint i = 0; i < numClauses; ++i){
    bool someTrue = false;
    for (uint j = 0; not someTrue and j < clauses[i].size(); ++j)
      someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
    if (not someTrue) {
      cout << "Error in model, clause is not satisfied:";
      for (uint j = 0; j < clauses[i].size(); ++j) cout << clauses[i][j] << " ";
      cout << endl;
      exit(1);
    }
  }  
}

int main(){ 
  readClauses(); // reads numVars, numClauses and clauses
  model.resize(numVars+1,UNDEF);
  indexOfNextLitToPropagate = 0;  
  decisionLevel = 0;
  
  // Take care of initial unit clauses, if any
  for (uint i = 0; i < numClauses; ++i)
    if (clauses[i].size() == 1) {
      int lit = clauses[i][0];
      int val = currentValueInModel(lit);
      if (val == FALSE) {cout << "UNSATISFIABLE" << endl; return 10;}
      else if (val == UNDEF) setLiteralToTrue(lit);
    }
    
  // DPLL algorithm
  while (true) {
    while ( propagateGivesConflict() ) {
      if ( decisionLevel == 0) { cout << "UNSATISFIABLE" << endl; return 10; }
      backtrack();
    }
    int decisionLit = getNextDecisionLiteral();
    if (decisionLit == 0) { checkmodel(); cout << "SATISFIABLE" << endl; return 20; }
    // start new decision level:
    modelStack.push_back(0);  // push mark indicating new DL
    ++indexOfNextLitToPropagate;
    ++decisionLevel;
    setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
  }
}  
