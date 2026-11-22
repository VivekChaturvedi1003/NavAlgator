#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

template <typename T>
class ManualQueue {
private:
    T* data;
    int capacity;
    int frontIndex;
    int count;

    void resize() {
        int newCapacity = capacity * 2;
        T* newData = new T[newCapacity];
        for (int i = 0; i < count; i++) {
            newData[i] = data[(frontIndex + i) % capacity];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
        frontIndex = 0;
    }

public:
    ManualQueue(int initialCapacity = 64) {
        capacity = initialCapacity;
        data = new T[capacity];
        frontIndex = 0;
        count = 0;
    }

    ~ManualQueue() {
        delete[] data;
    }

    void push(const T& value) {
        if (count == capacity) resize();
        int backIndex = (frontIndex + count) % capacity;
        data[backIndex] = value;
        count++;
    }

    T pop() {
        T value = data[frontIndex];
        frontIndex = (frontIndex + 1) % capacity;
        count--;
        return value;
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }
};

template <typename T>
class ManualStack {
private:
    T* data;
    int capacity;
    int count;

    void resize() {
        int newCapacity = capacity * 2;
        T* newData = new T[newCapacity];
        for (int i = 0; i < count; i++) newData[i] = data[i];
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    ManualStack(int initialCapacity = 32) {
        capacity = initialCapacity;
        data = new T[capacity];
        count = 0;
    }

    ~ManualStack() {
        delete[] data;
    }

    void push(const T& value) {
        if (count == capacity) resize();
        data[count] = value;
        count++;
    }

    T pop() {
        count--;
        return data[count];
    }

    void clear() {
        count = 0;
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }
};

struct PQNode {
    int row, col;
    int dist;     
};

class ManualMinHeap {
private:
    PQNode* data;
    int capacity;
    int count;

    void swapNodes(int i, int j) {
        PQNode temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }


    void bubbleUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (data[parent].dist <= data[i].dist) break;
            swapNodes(i, parent);
            i = parent;
        }
    }

    void bubbleDown(int i) {
        while (true) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;
            if (left < count && data[left].dist < data[smallest].dist) smallest = left;
            if (right < count && data[right].dist < data[smallest].dist) smallest = right;
            if (smallest == i) break;
            swapNodes(i, smallest);
            i = smallest;
        }
    }

    void resize() {
        int newCapacity = capacity * 2;
        PQNode* newData = new PQNode[newCapacity];
        for (int i = 0; i < count; i++) newData[i] = data[i];
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    ManualMinHeap(int initialCapacity = 64) {
        capacity = initialCapacity;
        data = new PQNode[capacity];
        count = 0;
    }

    ~ManualMinHeap() {
        delete[] data;
    }

    void push(PQNode node) {
        if (count == capacity) resize();
        data[count] = node;
        bubbleUp(count);
        count++;
    }

    PQNode popMin() {
        PQNode top = data[0];
        count--;
        data[0] = data[count];
        bubbleDown(0);
        return top;
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }
};

#endif
