#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <cstdlib>
#include <cmath>
#include "thread.h"

using namespace std;

struct Request {
    int requester_id;
    int track;
};

int max_disk_queue;
int current_track = 0;
int living_requesters;

list<Request> disk_queue;

unsigned int lock = 10;
unsigned int cv_queue_not_full = 20;
unsigned int cv_queue_has_request = 30;

void requester(void* arg) {
    auto args = (pair<int, string>*)arg;
    int requester_id = args->first;
    string filename = args->second;
    delete args;  // Delete here, once used safely.

    ifstream infile(filename);
    int track;

    while (infile >> track) {
        thread_lock(lock);
        while ((int)disk_queue.size() >= max_disk_queue) {
            thread_wait(lock, cv_queue_not_full);
        }

        disk_queue.push_back({requester_id, track});
        cout << "requester " << requester_id << " track " << track << endl;

        thread_broadcast(lock, cv_queue_has_request);

        // Wait until this request has been serviced
        thread_wait(lock, requester_id + 10); // unique cond variable per requester
        thread_unlock(lock);
    }

    thread_lock(lock);
    living_requesters--;
    thread_broadcast(lock, cv_queue_has_request);
    thread_unlock(lock);
}

void scheduler(void* arg) {
    auto args = (pair<int, vector<string>*>*)arg;
    max_disk_queue = args->first;
    vector<string> files = *(args->second);
    delete args->second;  // Safely delete after copying vector
    delete args;

    living_requesters = files.size();

    // Create requester threads
    for (int i = 0; i < (int)files.size(); i++) {
        thread_create(requester, new pair<int, string>(i, files[i]));
    }

    thread_lock(lock);
    current_track = 0;

    while (living_requesters > 0 || !disk_queue.empty()) {
        // Wait until enough requests are in the queue
        while ((int)disk_queue.size() < min(living_requesters, max_disk_queue) && living_requesters > 0) {
            thread_wait(lock, cv_queue_has_request);
        }

        if (disk_queue.empty()) {
            thread_unlock(lock);
            continue;
        }

        // Find closest request (SSTF)
        auto closest_it = disk_queue.begin();
        int min_dist = abs(closest_it->track - current_track);
        for (auto it = disk_queue.begin(); it != disk_queue.end(); ++it) {
            int dist = abs(it->track - current_track);
            if (dist < min_dist) {
                min_dist = dist;
                closest_it = it;
            }

        }

        Request req = *closest_it;
        disk_queue.erase(closest_it);

        // Service request
        current_track = req.track;
        cout << "service requester " << req.requester_id << " track " << req.track << endl;

        thread_signal(lock, req.requester_id + 10); // Signal specific requester
        thread_broadcast(lock, cv_queue_not_full);
    }

    thread_unlock(lock);
    cout << "Thread library exiting.\n";
    exit(0);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Usage: ./disk max_disk_queue disk.in0 disk.in1 ..." << endl;
        return 1;
    }

    max_disk_queue = atoi(argv[1]);
    vector<string>* files = new vector<string>();
    for (int i = 2; i < argc; i++)
        files->push_back(argv[i]);

    living_requesters = files->size();
    thread_libinit(scheduler, new pair<int, vector<string>*>(max_disk_queue, files));
}

