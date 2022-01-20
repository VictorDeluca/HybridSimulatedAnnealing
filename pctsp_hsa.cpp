//Algoritmo de Hybrid Simulated Annealing para o PCTSP com memoria tabu integrada. Utiliza uma funcao de Nearest Neighbour para gerar a solucao inicial.
#include <bits/stdc++.h>
#define LIM 1000
#define INF 0x3f3f3f3f
using namespace std;

//Memory
set<vector<int>> tabu_list; //To consult whether or not an element is in the list
deque<vector<int>> tabu_aux; //Check the next element to remove

typedef long long int lli;

lli graph[1000][1000];
lli prizes[1000],penalties[1000]; int visited[1000];
int v,e;
vector<int> cur_ans,ans,best_ans; 
lli cur_prize,prize,pmin;

void add_to_list(vector<int> v1){ //Adding an element to the tabu list
	tabu_list.insert(v1);
	tabu_aux.push_back(v1);
	if(tabu_list.size() > LIM){ //Respecting limit size of the tabu list
		vector<int> last = tabu_aux.front();
		tabu_aux.pop_front();
		tabu_list.erase(last);
	}
}

//Check if a solution breaks the packing constraint
int invalid(vector<int> path){
	int i, prize = 0;
	for(i=0;i<path.size() - 1;i++) prize += prizes[path[i]];

	if(prize < pmin) return 1;
	return 0;
}

//'Nearest Neighbour' recursiva: A cada passo, procura o vizinho mais proximo que ainda nao esta na solucao
void build_solution(int cur) {
	if (cur_ans.size() == v || cur_prize >= pmin)
		return;

	int i, j;
	int next = -1;
	for (i = 0; i < v; i++) {
		if (visited[i])
			continue;
		if (next == -1 || (graph[cur][i] <= graph[cur][next]) )
			next = i;
	}
	if(next == -1) return; //Not necessary, but just in case

	visited[next] = 1;
	cur_ans.push_back(next);
	cur_prize += prizes[next];

	build_solution(next);
}

vector<int> build_outside(vector<int> path){
	//Armazenamos todos os elementos FORA da solucao atual em outside[]
	int i,j; vector<int> outside;
	for(i=0;i<v;i++){
		for(j=0;j<path.size();j++) if(path[j] == i) break;

		if(j == path.size()) outside.push_back(i);
	}	
	return outside;
}

//Cost of a solution
lli get_value(vector<int> path) {
	int i; double ans = 0;
	for (i = 1; i < path.size(); i++)
		ans += graph[path[i - 1]][path[i]];

	vector<int> outside = build_outside(path);

	for(i=0;i<outside.size();i++)
		ans += penalties[outside[i]];
	
	return ans;
}

//Neighborhood function 1 (2-opt)
vector<int> f_2opt(int i, int j, vector<int> cur){
	int k;
	vector<int> new_ans;
	for(k=0;k<i;k++) new_ans.push_back(cur[k]);
	for(k=j;k>=i;k--) new_ans.push_back(cur[k]);
	for(k=j+1;k<(cur.size() - 1);k++) new_ans.push_back(cur[k]);

	new_ans.push_back(new_ans[0]);
	return new_ans;
}

//Neighborhood function 2 (Add to position)
vector<int> add_id(int pos, vector<int> cur){
	if(cur.size() == v) return cur;

	//Picking a vertex that isnt in the graph
	vector<int> outside = build_outside(cur);
	int id = rand()%outside.size();
	int ver = outside[id];

	vector<int> new_ans = cur;
	new_ans.insert(new_ans.begin() + pos,ver);

	new_ans[new_ans.size() - 1] = new_ans[0]; //So that cur_ans remains a cycle
	return new_ans;
}

//Neighborhood function 3 (Remove from position)
vector<int> remove_id(int pos, vector<int> cur){
	vector<int> new_ans = cur;
	new_ans.erase(new_ans.begin() + pos);
	new_ans[new_ans.size() - 1] = new_ans[0];

	if(invalid(new_ans)) return cur;
	else return new_ans;
}

int main() {
	int i,j,k;
	//Inicializando
	FILE *fin,*fout;
	fin = fopen("in.txt","r");
	fout = fopen("out.txt","w");
	
	srand (time(NULL));
	memset(visited,0,sizeof(visited));
	for (i = 0; i < 1000; i++)
		for (j = 0; j < 1000; j++)
			graph[i][j] = INF;
	
	//Lendo o grafo
	//Size
	fscanf(fin,"%d",&v);
	//Prizes
	for(i=0;i<v;i++){
		fscanf(fin,"%lld",&prizes[i]);
		pmin += prizes[i];
	}
	pmin *= 75;
	pmin /= 100;
	//Penalties
	for(i=0;i<v;i++)
		fscanf(fin,"%lld",&penalties[i]);
	//Edges
	for(i=0;i<v;i++)
		for(j=0;j<v;j++)
			fscanf(fin,"%lld",&graph[i][j]);

	//Descobrindo qual e o melhor ponto de partida
	for(i=0;i<v;i++){
		//Flushing data
		cur_prize = 0;
		memset(visited,0,sizeof(visited));
		cur_ans.clear();

		visited[i] = 1;
		cur_ans.push_back(i);
		build_solution(i);
		cur_ans.push_back(i);

		if(i==0 || (get_value(cur_ans) < get_value(ans)) ){
			ans = cur_ans;
			prize = cur_prize;
		}
	}

	cur_ans = ans;
	cur_prize = prize;
	
	//Implementacao da Simulated Annealing (Codigo reciclado de mim mesmo)
	double T = 100, it;
	best_ans = ans;

	//Best: Best solution
	//Ans: Current solution before modification
	//Cur_ans: Current solution after modification
	int collisions = 0, iterations = 0;
	while(T>0.1){ //Encerramos a execucao quando a temperatura for baixa o suficiente - no caso, 0.1
		it = 200; //Iteracoes para cada valor de tempetatura
		while(it--){
			//Choose neighborhood function
			int r1 = (rand()%3);
			if(r1 == 0){ //2-opt
				int v1 = 1, v2 = 1;
				while(v1==v2){
					v1 = rand()%(ans.size() - 1);
					v2 = rand()%(ans.size() - 1);
				}
				int lo = min(v1,v2), hi = max(v1,v2);
				cur_ans = f_2opt(lo,hi,ans);
			} else if(r1 == 1){ //Add
				int v1 = rand()%(ans.size() - 1);
				cur_ans = add_id(v1,ans);
			} else { //Remove
				int v1 = rand()%(ans.size() - 1);
				cur_ans = remove_id(v1,ans);
			}

			if(get_value(cur_ans) < get_value(ans)){ //Improvement found
				iterations++;
				if(tabu_list.find(cur_ans) == tabu_list.end()){
					add_to_list(cur_ans);
					ans = cur_ans;
					if(get_value(best_ans) > get_value(ans)) best_ans = ans; //New best solution
				} else collisions++; //Collision detected
			} else { //Melhoria nao encontrada
				double delta = get_value(cur_ans) - get_value(ans);
				double prob = exp(-delta/T); //Funcao de probabilidade
				double r1 = (rand()%1000); r1/=1000; //r1, nossa probabilidade, pode estar entre 0 e 1

				if(r1 <= prob){ //Probabilistic accept
					iterations++;
					if(tabu_list.find(cur_ans) != tabu_list.end()) collisions++; //Collision detected
					else{
						add_to_list(cur_ans);
						ans = cur_ans;
					}
				}
			}
		}

		T*=0.95;
	}

	//Solucao otima
	fprintf(fout,"The quickest route found was:\n");
	for (j = 0; j < best_ans.size(); j++)
		fprintf(fout,"%d ", best_ans[j]);
	fprintf(fout,"\n");
	fprintf(fout,"And its cost was %lld\n", get_value(best_ans));
	
	double percentage = (100.0 * collisions)/(1.0 * iterations);
	fprintf(fout,"%d collisions occurred, totalling %.4lf%% of the iterations\n",collisions,percentage);
	
	fclose(fin);
	fclose(fout);
}