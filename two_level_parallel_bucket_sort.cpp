// two-level parallel bucket sort
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace chrono;

void bucketSort(vector<int>& arr, int num_threads) {
    if (arr.empty()) return;

    int minVal = *min_element(arr.begin(), arr.end());
    int maxVal = *max_element(arr.begin(), arr.end());

    int range = maxVal - minVal;

    int numBuckets = static_cast<int>(ceil(static_cast<double>(range) / num_threads));
    int bucketSize = static_cast<int>(ceil(static_cast<double>(range) / numBuckets));

    vector<vector<int>> buckets(numBuckets);

    // parallel scatter
    omp_set_num_threads(num_threads);
#pragma omp parallel
    {
        vector<vector<int>> local_buckets(numBuckets);

#pragma omp for nowait
        for (int i = 0; i < arr.size(); ++i) {
            int bucketIndex = (arr[i] - minVal) / bucketSize;
            if (bucketIndex >= numBuckets) bucketIndex = numBuckets - 1;
            local_buckets[bucketIndex].push_back(arr[i]);
        }

#pragma omp critical
        for (int i = 0; i < numBuckets; ++i) {
            buckets[i].insert(buckets[i].end(), local_buckets[i].begin(), local_buckets[i].end());
        }
    }

    // parallel sort 
#pragma omp parallel for
    for (int i = 0; i < numBuckets; ++i) {
        sort(buckets[i].begin(), buckets[i].end());
    }

    // gather
    arr.clear();

    for (const auto& bucket : buckets) {
        arr.insert(arr.end(), bucket.begin(), bucket.end());
    }
}

int main() {
    vector<int> arr;
    int num_elements = 100000;
    int num_threads = 4;
    int range_elements = 1000;

    srand(time(0));
    for (int i = 0; i < num_elements; ++i) {
        arr.push_back(rand() % range_elements); 
    }


    auto start = high_resolution_clock::now();

    bucketSort(arr, num_threads);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Number of threads: " << num_threads << endl;
    cout << "Execution time (PARALLEL): " << duration.count() << " microseconds" << endl;
    cout << "Number of elements sorted: " << arr.size() << endl;
    cout << "Range of elements sorted: " << "0 to " << range_elements << endl;

    //for (const auto& elem : arr) cout << elem << " ";
    cout << endl;

    return 0;
}

