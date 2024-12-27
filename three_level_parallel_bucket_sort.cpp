// three-level parallel bucket sort
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

void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void parallelMergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

#pragma omp parallel sections
        {
#pragma omp section
            parallelMergeSort(arr, left, mid);

#pragma omp section
            parallelMergeSort(arr, mid + 1, right);
        }

        merge(arr, left, mid, right);
    }
}

void bucketSort(vector<int>& arr, int num_threads) {
    if (arr.empty()) return;

    int minVal = *min_element(arr.begin(), arr.end());
    int maxVal = *max_element(arr.begin(), arr.end());

    int range = maxVal - minVal;

    int numBuckets = static_cast<int>(ceil(static_cast<double>(range) / num_threads));
    int bucketSize = std::max(1, static_cast<int>(ceil(static_cast<double>(range) / numBuckets)));

    vector<vector<int>> buckets(numBuckets);

    // scatter
    omp_set_num_threads(num_threads);
#pragma omp parallel
    {
        vector<vector<int>> local_buckets(numBuckets);

#pragma omp for nowait
        for (size_t i = 0; i < arr.size(); ++i) {
            int bucketIndex = (arr[i] - minVal) / bucketSize;
            if (bucketIndex >= numBuckets) bucketIndex = numBuckets - 1;
            local_buckets[bucketIndex].push_back(arr[i]);
        }

#pragma omp critical
        for (int i = 0; i < numBuckets; ++i) {
            buckets[i].insert(buckets[i].end(), local_buckets[i].begin(), local_buckets[i].end());
        }
    }

    // sort
#pragma omp parallel for
    for (int i = 0; i < numBuckets; ++i) {
        parallelMergeSort(buckets[i], 0, buckets[i].size() - 1);
    }

    // gather
    arr.clear();
    for (const auto& bucket : buckets) {
        arr.insert(arr.end(), bucket.begin(), bucket.end());
    }
}

int main() {
    vector<int> arr;
    int num_elements = 10;
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
    cout << "Execution time (PARALLEL MERGE SORT): " << duration.count() << " microseconds" << endl;
    cout << "Number of elements sorted: " << arr.size() << endl;
    cout << "Range of elements sorted: " << "0 to " << range_elements << endl;

    //for (const auto& elem : arr) cout << elem << " ";
    cout << endl;

    return 0;
}
