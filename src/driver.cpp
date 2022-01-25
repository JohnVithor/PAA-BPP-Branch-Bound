// driver.cpp

#include <chrono>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <set>
#include <stack>
#include <algorithm>
#include <random>

typedef struct ProblemInstance
{
    size_t C;
    size_t N;
    size_t* items;
} ProblemInstance;

typedef struct Solution
{
    size_t k;
    size_t N;
    size_t* items_bins;
    size_t* bins_c;
} Solution;

typedef struct CandidateSolutionTree
{
    Solution* solution;
    size_t next_item_index;
} CandidateSolutionTree;

typedef struct SolutionAndMetrics
{
    Solution* solution;
    size_t n_nodes_open;
    size_t n_cuts;
    size_t n_equivalents_ignored;
} SolutionAndMetrics;

Solution* copy(Solution* sol) {
    Solution* result = new Solution;
    result->N = sol->N;
    result->k = sol->k;
    result->items_bins = new size_t[sol->N];
    result->bins_c = new size_t[sol->N];
    for (size_t i = 0; i < sol->N; ++i) {
        result->items_bins[i] = sol->items_bins[i];
        result->bins_c[i] = sol->bins_c[i];
    }
    return result;
}

CandidateSolutionTree* createCandidateSolutionTree(Solution* sol, CandidateSolutionTree* previous)
{
    CandidateSolutionTree* child = new CandidateSolutionTree;
    child->solution = sol;
    child->next_item_index = previous->next_item_index+1;
    return child;
}

bool decreasing(size_t i, size_t j) { return (i>j); }

Solution* best_fit(ProblemInstance* instance) {
    Solution *s = new Solution;
    s->k = 0;
    s->N = instance->N;
    s->items_bins = new size_t[instance->N];
    s->bins_c = new size_t[instance->N];
    std::fill_n(s->bins_c,instance->N,0);
    std::fill_n(s->items_bins,instance->N,0);
    size_t largest_content_i = instance->N;
    size_t largest_content = 0;
    for (size_t i = 0; i < instance->N; ++i)
    {
        largest_content_i = instance->N;
        largest_content = 0;
        for (size_t j = 0; j < s->k; ++j)
        {
            if (s->bins_c[j] + instance->items[i] <= instance->C && s->bins_c[j] > largest_content) {
                largest_content_i = j;
                largest_content = s->bins_c[j];
            }
        }
        if (largest_content_i != instance->N) {
            s->bins_c[largest_content_i] += instance->items[i];
            s->items_bins[i] = largest_content_i+1;
        } else {
            s->bins_c[s->k] = instance->items[i];
            ++s->k;
            s->items_bins[i] = s->k;
        }
    }
    return s;
}

Solution* best_fit_decreasing(ProblemInstance* instance) {
    std::sort(instance->items, instance->items+instance->N, decreasing);
    return best_fit(instance);
}


std::stack<CandidateSolutionTree*> populate_candidates(ProblemInstance* instance)
{
    Solution *s = new Solution;
    s->k = 0;
    s->N = instance->N;
    s->items_bins = new size_t[instance->N];
    s->bins_c = new size_t[instance->N];
    std::fill_n(s->bins_c,instance->N,0);
    std::fill_n(s->items_bins,instance->N,0);
    size_t start_packing = 0;
    for (start_packing = 0; start_packing < instance->N; ++start_packing)
    {
        if(s->bins_c[s->k] + instance->items[start_packing] > instance->C/2) {
            s->bins_c[s->k] = instance->items[start_packing];
            ++s->k;
            s->items_bins[start_packing] = s->k;
        } else {
            break;
        }
    }
    CandidateSolutionTree* candidate = new CandidateSolutionTree;
    candidate->solution = s;
    candidate->next_item_index = start_packing;
    std::stack<CandidateSolutionTree*> result;
    result.push(candidate);
    return result;
    
}

size_t L1(ProblemInstance* instance) {
    size_t total = 0;
    for (size_t i = 0; i < instance->N; ++i)
    {
        total += instance->items[i];
    }
    return std::ceil(total / (double) instance->C);
}

size_t Lk(ProblemInstance* instance, size_t k) {
    std::vector<size_t> n1;
    std::vector<size_t> n2;
    long sum_n2 = 0;
    std::vector<size_t> n3;
    long sum_n3 = 0;
    const size_t n1_cut = instance->C - k;
    const size_t C_half = instance->C / 2;
    for (size_t i = 0; i < instance->N; ++i){
        if (instance->items[i] > n1_cut) {
            n1.push_back(instance->items[i]);
        } else {
            if (instance->items[i] > C_half) {
                n2.push_back(instance->items[i]);
                sum_n2 +=instance->items[i];
            } else {
                if(instance->items[i] >= k) {
                    n3.push_back(instance->items[i]);
                    sum_n3 +=instance->items[i];
                }
            }
        }
    }
    const long last_value = std::max(0.0, std::ceil((sum_n3 - (long)(n2.size()*instance->C - sum_n2)) / (double) instance->C));
    return n1.size() + n2.size() + last_value;
}

size_t L2(ProblemInstance* instance) {
    std::vector<size_t> n1;
    std::vector<size_t> n2;
    std::set<size_t> distinct_values;
    const size_t C_half = instance->C / 2;
    for (size_t i = 0; i < instance->N; ++i){
        if (instance->items[i] > C_half) {
            n1.push_back(instance->items[i]);
        } else {
            distinct_values.insert(instance->items[i]);
        }
    }
    if (distinct_values.empty()) {
        return n1.size();
    } else {
        size_t result = 0;
        for (auto it = distinct_values.begin(); it != distinct_values.end(); ++it) {
            result = std::max(result, Lk(instance, *it));
        }
        return result;
    }
}

size_t lower_bound_function(CandidateSolutionTree* candidate, ProblemInstance* instance) {
    size_t sum_of_capacity_items = 0;
    for (size_t j = 0; j < candidate->solution->k; ++j) {
        bool has_valid_space = false;
        for (size_t i = candidate->next_item_index; i < instance->N; ++i) {
            if (candidate->solution->bins_c[j] + instance->items[i] <= instance->C) {
                has_valid_space = true;
                break;
            }
        }
        if (has_valid_space) {
            sum_of_capacity_items += candidate->solution->bins_c[j];
        } else {
            sum_of_capacity_items += instance->C;
        }
    }
    for (size_t i = candidate->next_item_index; i < instance->N; ++i) {
        sum_of_capacity_items += instance->items[i];
    }
    const size_t result = (size_t) std::ceil(sum_of_capacity_items/ (double) instance->C);
    return std::max(result, candidate->solution->k);
}

SolutionAndMetrics* branch_and_bound_solve(ProblemInstance* instance, const size_t lower_bound_id, const long int max_time_min) {
    CandidateSolutionTree* current_optimum = new CandidateSolutionTree;
    current_optimum->solution = best_fit_decreasing(instance);
    size_t current_upper_bound = current_optimum->solution->k;
    size_t lower_bound = instance->N;
    if (lower_bound_id == 1)
    {
        lower_bound = L1(instance);
    } 
    else if (lower_bound_id == 2)
    {
        lower_bound = L2(instance);
    }
    else
    {
        std::exit(1);
    }
    
    if (current_upper_bound == lower_bound) {
        SolutionAndMetrics* results = new SolutionAndMetrics;
        results->solution = current_optimum->solution;
        results->n_nodes_open = 0;
        results->n_cuts = 0;
        results->n_equivalents_ignored = 0;
        return results;
    }
    std::stack<CandidateSolutionTree*> candidate_stack = populate_candidates(instance);
    size_t n_nodes_open = 0;
    size_t n_cuts = 0;
    size_t n_equivalents_ignored = 0;
    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (!candidate_stack.empty()) {
        const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::minutes>(end-begin).count() > max_time_min) {
            while (!candidate_stack.empty())
            {
                CandidateSolutionTree* to_delete = candidate_stack.top();
                candidate_stack.pop();
                delete[] to_delete->solution->bins_c;
                delete[] to_delete->solution->items_bins;
                delete to_delete->solution;
                delete to_delete;
            }
            SolutionAndMetrics* results = new SolutionAndMetrics;
            results->solution = current_optimum->solution;
            results->n_nodes_open = n_nodes_open;
            results->n_cuts = n_cuts;
            results->n_equivalents_ignored = n_equivalents_ignored;
            return results;
        }
        ++n_nodes_open;
        CandidateSolutionTree* node = candidate_stack.top();
        candidate_stack.pop();
        if (node->next_item_index == instance->N) {
            if (node->solution->k < current_upper_bound) {
                delete[] current_optimum->solution->bins_c;
                delete[] current_optimum->solution->items_bins;
                delete current_optimum->solution;
                delete current_optimum;
                current_optimum = node;
                current_upper_bound = node->solution->k;
                if (current_upper_bound == lower_bound) {
                    while (!candidate_stack.empty())
                    {
                        CandidateSolutionTree* to_delete = candidate_stack.top();
                        candidate_stack.pop();
                        delete[] to_delete->solution->bins_c;
                        delete[] to_delete->solution->items_bins;
                        delete to_delete->solution;
                        delete to_delete;
                    }
                    SolutionAndMetrics* results = new SolutionAndMetrics;
                    results->solution = current_optimum->solution;
                    results->n_nodes_open = n_nodes_open;
                    results->n_cuts = n_cuts;
                    results->n_equivalents_ignored = n_equivalents_ignored;
                    return results;
                }           
            } else {
                delete[] node->solution->bins_c;
                delete[] node->solution->items_bins;
                delete node->solution;
                delete node;
            }
        } else {
            std::vector<std::set<size_t>> bins_items(node->solution->k);
            for (size_t i = 0; i < node->next_item_index; ++i) {
                bins_items[node->solution->items_bins[i]-1].insert(instance->items[i]);
            }
            std::set<std::set<size_t>> valid_bins;
            for (size_t i = 0; i < bins_items.size(); ++i) {
                valid_bins.insert(bins_items[i]);
            }
            std::vector<CandidateSolutionTree*> children;
            for (size_t j = 0; j < node->solution->k; ++j) {
                size_t bin_new_value = node->solution->bins_c[j] + instance->items[node->next_item_index];
                if (bin_new_value <= instance->C) {
                    if (valid_bins.find(bins_items[j]) != valid_bins.end()){
                        valid_bins.erase(bins_items[j]);
                        Solution* sol = copy(node->solution);
                        sol->bins_c[j] += instance->items[node->next_item_index];
                        sol->items_bins[node->next_item_index] = j+1;
                        children.push_back(createCandidateSolutionTree(sol, node));
                    } else {
                        ++n_equivalents_ignored;
                    }
                }
            }
            valid_bins.clear();
            bins_items.clear();
            Solution* sol = copy(node->solution);
            sol->bins_c[node->solution->k] = instance->items[node->next_item_index];
            sol->k = node->solution->k+1;
            sol->items_bins[node->next_item_index] = sol->k;
            children.push_back(createCandidateSolutionTree(sol, node));

            for (size_t i = 0; i < children.size(); ++i)
            {
                if (lower_bound_function(children.at(i), instance) < current_upper_bound) {
                    candidate_stack.push(children.at(i));
                } else {
                    CandidateSolutionTree* to_delete = children.at(i);
                    delete[] to_delete->solution->bins_c;
                    delete[] to_delete->solution->items_bins;
                    delete to_delete->solution;
                    delete to_delete;
                    ++n_cuts;
                }
            }
            children.clear();
            delete[] node->solution->bins_c;
            delete[] node->solution->items_bins;
            delete node->solution;
            delete node;
        }
    }
    SolutionAndMetrics* results = new SolutionAndMetrics;
    results->solution = current_optimum->solution;
    results->n_nodes_open = n_nodes_open;
    results->n_cuts = n_cuts;
    results->n_equivalents_ignored = n_equivalents_ignored;
    return results;
}

int main(int argc, char const *argv[]) {
    if (argc != 6) {
        std::cout << "O programa deve receber o caminho para arquivo com os dados da instância." << std::endl;
        std::cout << "A seed para realizar o shuffle dos elementos antes de aplicar o algoritmo escolhido (Os items já estão ordenados nos arquivos)" << std::endl;
        std::cout << "Indicar se deve ser usado o lower bound L1 ou L2 (1 ou 2)" << std::endl;
        std::cout << "Em seguida o tempo máximo em minutos que o programa pode executar até encontrar o resultado ótimo" << std::endl;
        std::cout << "E por fim o nível de detalhe a ser informado na saida padrão sobre os resultados obtidos (0-6)" << std::endl;
        std::cout << "0 - Apenas a quantidade de bins usada e o tempo em nanosegundos" << std::endl;
        std::cout << "1 - O anterior e o tempo em segundos" << std::endl;
        std::cout << "2 - O anterior e o tempo em minutos" << std::endl;
        std::cout << "3 - O anterior e a quantidade de nós explorados" << std::endl;
        std::cout << "4 - O anterior e a quantidade de cortes realizados" << std::endl;
        std::cout << "5 - O anterior e a quantidade de nós ignorados por serem equivalentes a outros já adicionados a pilha de exploração" << std::endl;
        std::cout << "6 - O anterior e em qual bin cada item foi empacotado além da capacidade de cada bin" << std::endl;
        exit(1);
    }
    std::ifstream fileSource(argv[1]);
    size_t N = 0;
    size_t C = 0;
    size_t* items = nullptr;
    if (!fileSource) {
        std::cerr << "Erro: Arquivo não pode ser aberto - " << argv[1] << std::endl;
        return 1;
    } else {
        fileSource >> N;
        fileSource >> C;
        items = new size_t[N];
        for (size_t i = 0; i < N; ++i)
        {
            fileSource >> items[i];
        }
        fileSource.close();
    }
    std::shuffle(items, items+N, std::default_random_engine(std::strtoul(argv[2], nullptr, 10)));
    const long int lower_bound_id = std::strtoul(argv[3], nullptr, 10);
    if (!(lower_bound_id == 1 || lower_bound_id == 2)) {
        std::cout << "O lower bound deve ser identificado apenas com '1' ou com '2'" << std::endl;
        return 1;
    }
    const long int max_time_min = std::strtoul(argv[4], nullptr, 10);
    if (!(max_time_min > 0)) {
        std::cout << "O tempo máximo em minutos deve ser maior que 0" << std::endl;
        return 1;
    }
    const long int detail_level = std::strtoul(argv[5], nullptr, 10);
    if (!(detail_level > -1 && detail_level < 7)) {
        std::cout << "O nível de detalhe deve ser qualquer inteiro entre {0,1,2,3,4,5,6}" << std::endl;
        return 1;
    }
    ProblemInstance instance;
    instance.C = C;
    instance.N = N;
    instance.items = items;
    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    SolutionAndMetrics* s = branch_and_bound_solve(&instance, lower_bound_id, max_time_min);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Número de bins: " << s->solution->k << std::endl;
    std::cout << "Tempo em nanosegundos: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << std::endl;
    if (detail_level > 0) {
        std::cout << "Tempo em segundos: " << std::chrono::duration_cast<std::chrono::seconds>(end-begin).count() << std::endl;    
        if (detail_level > 1) {
            std::cout << "Tempo em minutos: " << std::chrono::duration_cast<std::chrono::minutes>(end-begin).count() << std::endl;
            if (detail_level > 2) {
                std::cout << "Número total de nós explorados: " << s->n_nodes_open << std::endl;
                if (detail_level > 3) {
                    std::cout << "Quantidade de cortes realizados: " << s->n_cuts << std::endl;
                    if (detail_level > 4) {
                        std::cout << "Quantidade de nós ignorados por serem equivalentes a outros: " << s->n_equivalents_ignored << std::endl;
                        if (detail_level > 5) {
                            std::cout << std::left;
                            for (size_t i = 0; i < s->solution->N; ++i)
                            {
                                std::cout << "Item " << std::setw(4) << i+1 << " de valor " << std::setw(4) << items[i] << " na bin " << std::setw(4) << s->solution->items_bins[i] << std::endl;
                            }
                            std::cout << "Capacidade por bin: " << C << std::endl;
                        }
                    }
                }
            }
        }
    }
    delete[] items;
    delete[] s->solution->bins_c;
    delete[] s->solution->items_bins;
    delete s->solution;
    delete s;
    return 0;
}