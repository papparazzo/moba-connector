/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2019 Stefan Paproth <pappi-@gmx.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/agpl.txt>.
 *
 */

#pragma once

#include <mutex>
#include <exception>
#include <utility>
#include <queue>
#include <condition_variable>

class TerminationException : public std::exception {

    public:
        virtual ~TerminationException() noexcept {
        }

        TerminationException(const std::string &what) {
            this->what__ = what;
        }

        virtual const char* what() const noexcept {
            return this->what__.c_str();
        }

    private:
        std::string what__;
};

template<typename T>
class ConcurrentQueue {
    public:
        virtual ~ConcurrentQueue() noexcept {
        }

        void push(const T &data) {
            std::unique_lock<std::mutex> lock(mutex);
            if (terminate) {
                throw TerminationException("canceled");
            }
            queue.push(data);
            lock.unlock();
            condition.notify_one();
        }

        void push(const T &&data) {
            std::unique_lock<std::mutex> lock(mutex);
            if (terminate) {
                throw TerminationException("canceled");
            }
            queue.push(data);
            lock.unlock();
            condition.notify_one();
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(mutex);
            if (terminate) {
                throw TerminationException("canceled");
            }
            return queue.empty();
        }

        T pop() {
            std::unique_lock<std::mutex> lock(mutex);
            while(queue.empty()) {
                condition.wait(lock);
            }

            T value = std::move(queue.front());
            queue.pop();
            return value;
        }

        void terminate() {
            std::unique_lock<std::mutex> lock(mutex);
            terminate = true;
            lock.unlock();
            condition.notify_all();
        }

    private:
        std::queue<T> queue;
        std::mutex mutex;
        std::condition_variable condition;
        bool terminate;

        ConcurrentQueue(const ConcurrentQueue &orig) = delete;
};
