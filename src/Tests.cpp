#include "PMEM/Tests.hpp"
#include <stdio.h>
#include "GraphUtils.hpp"
#include <random>
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include "FormatUtils.hpp"
#include "BenchmarkUtils.hpp"

namespace Tests {
    void PMEM_tests() {
        printf("First test, simple struct write and read.\n");
        PMEM::Tests::libpmemobj_example_write_1();
        PMEM::Tests::libpmemobj_example_read_1();

        printf("Second test, simple struct write and read with type safety.\n");
        PMEM::Tests::libpmemobj_example_write_2();
        PMEM::Tests::libpmemobj_example_read_2();

        printf("Third test, this is using an API that I made.\n");
        PMEM::Tests::pmem_as_volatile_API();
    }

    void graph_test() {
        const uint32_t num_vertices = 4;
        const uint32_t min_degree = 1;
        const uint32_t max_degree = 2;
        const float min_value = 1;
        const float max_value = 2;

        std::string graph_file_path = "./graph_examples/test_format.csv";
        std::string simple_graph_path = "./graph_examples/simple_graph.csv";

        printf("First Graph\n");
        PMEM::GraphCRS g1 = GraphUtils::create_graph_crs_pmem("/tmp/", num_vertices, min_degree, max_degree, min_value, max_value);
        g1.print();
        g1.save(graph_file_path);
        g1.free();

        printf("Second Graph\n");
        PMEM::GraphCRS g2 = GraphUtils::load_as_pmem(simple_graph_path, "/tmp/");
        g2.print();
        std::vector<uint32_t> shortest_path = g2.shortest_path(2, 4);

        if (shortest_path.size() > 0 && shortest_path.size() <= 5) {

            printf("[");
            for (int i = 0; i < shortest_path.size() - 1; i++) {
                printf(" %u ->", shortest_path[i]);
            }
            printf(" %u ]\n", shortest_path.back());
        }
        else {
            printf("Shortest Path has wrong size [%lu]\n", shortest_path.size());
        }
        g2.free();
    }

    void graph_test_page_rank() {
        const uint32_t num_vertices = 1e9;
        const uint32_t min_degree = 1;
        const uint32_t max_degree = 10;
        const float min_value = 1;
        const float max_value = 5;
        const uint32_t iterations = 100;
        const float dampening_factor = 0.8;
        const uint32_t test_iterations = 5;

        printf("Testing Page Rank\n");
        printf("\tNumber of Vertices: %u\n", num_vertices);
        printf("\tMinimum Degree: %u\n", min_degree);
        printf("\tMaximum Degree: %u\n", max_degree);
        printf("\tIterations: %u\n", iterations);
        printf("\tDampening Factor: %f\n", dampening_factor);
        printf("\tTest Iterations: %u\n", test_iterations);

        std::string temp_graph_path = "./tmp/page_rank_graph.csv";

        {
            printf("Graph DRAM\n");
            GraphCRS graph = GraphUtils::create_graph_crs(num_vertices, min_degree, max_degree, min_value, max_value);
            graph.save(temp_graph_path);
            std::vector<int64_t> time_elapsed_v;
            for (uint32_t i = 0; i < test_iterations; i++)
            {
                Timer timer;
                graph.page_rank(iterations, dampening_factor);
                timer.end();
                time_elapsed_v.push_back(timer.get_time_elapsed());
                double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
                printf("\tTime Elapsed: %7.3f s", time_elapsed_seconds);
                printf("\tEdges per second: %7.3f\n", graph.num_edges() / time_elapsed_seconds);
            }
            int64_t min;
            int64_t max;
            double avg;
            double std_dev;
            BenchmarkUtils::metrics(time_elapsed_v, min, max, avg, std_dev);

            printf("\tTime Elapsed\n\t\tmin=%.3f\n\t\tmax=%.3f\n\t\tavg=%.3f\n\t\tstd_dev=%.3f\n", min / 1e9, max / 1e9, avg / 1e9, std_dev / 1e9);
            printf("\tEdges per second\n\t\tmin=%.3f\n\t\tmax=%.3f\n\t\tavg=%.3f\n", graph.num_edges() / (min / 1e9), graph.num_edges() / (max / 1e9), graph.num_edges() / (avg / 1e9));
        }

        printf("Graph PMEM\n");
        PMEM::GraphCRS graph_pmem = GraphUtils::load_as_pmem(temp_graph_path, "/pmem/");
        std::vector<int64_t> time_elapsed_v;
        for (uint32_t i = 0; i < test_iterations; i++)
        {
            Timer timer("Page Rank PMEM");
            graph_pmem.page_rank(iterations, dampening_factor);
            timer.end();
            time_elapsed_v.push_back(timer.get_time_elapsed());
            double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
            printf("\tTime Elapsed: %7.3f s", time_elapsed_seconds);
            printf("\tEdges per second: %7.3f\n", graph_pmem.num_edges() / time_elapsed_seconds);
        }
        int64_t min;
        int64_t max;
        double avg;
        double std_dev;
        BenchmarkUtils::metrics(time_elapsed_v, min, max, avg, std_dev);
        printf("\tTime Elapsed\n\t\tmin=%.3f\n\t\tmax=%.3f\n\t\tavg=%.3f\n\t\tstd_dev=%.3f\n", min / 1e9, max / 1e9, avg / 1e9, std_dev / 1e9);
        printf("\tEdges per second\n\t\tmin=%.3f\n\t\tmax=%.3f\n\t\tavg=%.3f\n", graph_pmem.num_edges() / (min / 1e9), graph_pmem.num_edges() / (max / 1e9), graph_pmem.num_edges() / (avg / 1e9));
        graph_pmem.free();
    }

    void memory_benchmark(char* arr, const size_t size) {

        /*
            TODO(EMU):
            Also do a measurement for latency
        */
        int iter_per_test = 10;
        printf("Memory Benchmark\n");
        printf("Memory Size: %sB\n", FormatUtils::format_number(size).c_str());
        printf("Iterations per Test: %d\n", iter_per_test);

        BlockTimer timer("Memory Test");
        uint64_t sum = 0;
        double size_gigabytes = size / 1e9;
        auto print_stat = [&size_gigabytes](const Timer& timer) {
            double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
            printf("\tTime Elapsed: %7.3f s", time_elapsed_seconds);
            printf("\tBandwidth: %7.3f GB/s\n", size_gigabytes / time_elapsed_seconds);
        };

        auto print_stat_vector = [&size_gigabytes](const std::vector<int64_t> time_elapsed_v) {
            int64_t min;
            int64_t max;
            double avg;
            double std_dev;
            BenchmarkUtils::metrics(time_elapsed_v, min, max, avg, std_dev);

            printf("\tTime Elapsed:\n\t\tmin=%.3f\n\t\tmax=%.3f\n\t\tavg=%.3f\n\t\tstd_dev=%.3f\n", min / 1e9, max / 1e9, avg / 1e9, std_dev / 1e9);
            printf("\tBandwidth:\n\t\tmin=%.3f\n\t\tmax=%.3f\n\t\tavg=%.3f\n", size_gigabytes / (max / 1e9), size_gigabytes / (min / 1e9), size_gigabytes / (avg / 1e9));
        };

        /* Read linear */
        {
            printf("Read Linear\n");
            std::vector<int64_t> time_elapsed_v;
            for (int iter = 0; iter < iter_per_test; iter++) {
                Timer timer;
#pragma omp parallel for reduction(+:sum)
                for (uint64_t i = 0; i < size; i++) {
                    sum += arr[i];
                }
                timer.end();
                print_stat(timer);
                time_elapsed_v.push_back(timer.get_time_elapsed());
            }
            print_stat_vector(time_elapsed_v);
            printf("IGNORE(%lu)\n", sum);
        }

        /* Read Random */
        {
            printf("Read Random\n");
            std::vector<int64_t> time_elapsed_v;
            for (int iter = 0; iter < iter_per_test; iter++) {
                Timer timer;
                sum = 0;

#pragma omp parallel
                {
                    std::default_random_engine generator;
                    std::uniform_int_distribution<uint64_t> distribution(0, size);
                    auto indexGen = std::bind(distribution, generator);

#pragma omp for reduction(+:sum)
                    for (uint64_t i = 0; i < size; i++) {
                        sum += arr[indexGen()];
                    }
                }
                timer.end();
                print_stat(timer);
                time_elapsed_v.push_back(timer.get_time_elapsed());
            }
            print_stat_vector(time_elapsed_v);
            printf("IGNORE(%lu)\n", sum);
        }

        /* Write linear */
        {
            printf("Write Linear\n");
            std::vector<int64_t> time_elapsed_v;
            for (int iter = 0; iter < iter_per_test; iter++) {
                Timer timer;
#pragma omp parallel for
                for (uint64_t i = 0; i < size; i++) {
                    arr[i] = 0;
                }
                timer.end();
                print_stat(timer);
                time_elapsed_v.push_back(timer.get_time_elapsed());
            }
            print_stat_vector(time_elapsed_v);
        }

        /* Write Random */
        {
            printf("Write Random\n");
            std::vector<int64_t> time_elapsed_v;
            for (int iter = 0; iter < iter_per_test; iter++) {
                Timer timer;
#pragma omp parallel
                {
                    std::default_random_engine generator;
                    std::uniform_int_distribution<uint64_t> distribution(0, size);
                    auto indexGen = std::bind(distribution, generator);

#pragma omp for
                    for (uint64_t i = 0; i < size; i++) {
                        arr[indexGen()] = 0;
                    }
                }
                timer.end();
                print_stat(timer);
                time_elapsed_v.push_back(timer.get_time_elapsed());
            }
            print_stat_vector(time_elapsed_v);
        }

        printf("IGNORE(%lu)\n", sum);
    }

    void pmem_vs_dram_benchmark(const size_t alloc_size) {
        printf("Allocation Size: %sB\n", FormatUtils::format_number(alloc_size).c_str());

        {
            printf("DRAM\n");
            char* array = new char[alloc_size];
            printf("IGNORE(%c)\n", array[0]);
            memory_benchmark(array, alloc_size);
            printf("IGNORE(%c)\n", array[0]);
            delete array;
        }

        {
            printf("Persistent Memory\n");
            PMEM::ptr pmem = PMEM::ptr("/pmem/", PMEM::FILE::TEMP, alloc_size);
            char* array = pmem.as<char*>();

            printf("Is persistent: %s\n", pmem.is_persistent() ? "True" : "False");
            printf("Mapped length: %sB\n", FormatUtils::format_number(pmem.mapped_len()).c_str());

            if (array == nullptr) {
                printf("Trouble allocating persistent memory\n");
                return;
            }
            printf("IGNORE(%c)\n", array[0]);
            memory_benchmark(array, alloc_size);
            printf("IGNORE(%c)\n", array[0]);
            pmem.free();
        }
    }
}