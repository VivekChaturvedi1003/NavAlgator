#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

// ---------------------------------------------------------------
// Manually implemented Queue (used by BFS in Enemy.cpp)
// Built on a dynamic array instead of std::queue, so the actual
// enqueue/dequeue mechanics are visible and are what gets evaluated.
// ---------------------------------------------------------------
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

// ---------------------------------------------------------------
// Manually implemented Stack (used for the Undo feature)
// Built on a dynamic array instead of std::stack.
// ---------------------------------------------------------------
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

    // Empties the stack (used when a round restarts so Undo can't jump back
    // to positions from the previous round).
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

#endif
