#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <utility>
#include <queue>

namespace NanoTask {

    /**
    * \brief Returns the current timestamp in nanoseconds.
    *
    * This function retrieves the current time using the high-resolution clock
    * and converts it to nanoseconds since the epoch.
    *
    * \return The current timestamp in nanoseconds.
    */
    static inline std::chrono::nanoseconds CurrNanoTimeStamp() {
        auto now = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
    }

    class Task {
    public:
        /**
         * \brief Constructs a Task object with the specified duration, function, and arguments.
         *
         * \tparam DurType     The type of the duration.
         * \tparam Func        The type of the function to be bound.
         * \tparam BoundArgs   The types of the arguments to be bound.
         * \param itrvl       The duration at which the task should be executed.
         * \param func        The function to be bound.
         * \param args        The arguments to be bound.
         */
        template<typename DurType, class Func, class... BoundArgs>
        inline Task(std::chrono::duration<DurType> itrvl, Func&& func, BoundArgs&&... args)
        {
            mHasSetInterval = false;

            mTask = [func = std::forward<Func>(func), args = std::make_tuple(args...)]() {
                std::apply(func, args);
            };

            setInterval(itrvl);
        }

        /**
         * \brief Sets the interval for the task execution in seconds.
         *
         * This function allows you to specify the interval at which the task should
         * be executed using a duration in seconds. The interval determines how often
         * the task will be executed after the initial execution.
         *
         * \param secs The interval duration in seconds.
         */
        inline void setIntervalSecs(unsigned long long secs)
        {
            setInterval(std::chrono::seconds(secs));
        }

        /**
         * \brief Sets the interval for the task execution in milliseconds.
         *
         * This function allows you to specify the interval at which the task should
         * be executed using a duration in milliseconds. The interval determines how often
         * the task will be executed after the initial execution.
         *
         * \param millis The interval duration in milliseconds.
         */
        inline void setIntervalMillis(unsigned long long millis)
        {
            setInterval(std::chrono::milliseconds(millis));
        }

        /**
         * \brief Sets the interval for the task execution in nanoseconds.
         *
         * This function allows you to specify the interval at which the task should
         * be executed using a duration in nanoseconds. The interval determines how often
         * the task will be executed after the initial execution.
         *
         * \param millis The interval duration in nanoseconds.
         */
        inline void setIntervalNanos(unsigned long long nanos)
        {
            setInterval(std::chrono::nanoseconds(nanos));
        }

        /**
         * \brief Sets the interval for the task execution using a custom duration.
         *
         * This function allows you to specify the interval at which the task should
         * be executed using a custom duration. The interval determines how often the
         * task will be executed after the initial execution.
         *
         * \param intervl The interval duration of type T.
         *
         * \tparam T The type of the interval duration. It must be a valid duration type
         *           compatible with std::chrono::nanoseconds.
         */
        template<typename T>
        inline void setInterval(T intervl)
        {
            setIntervalChronoNanos(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    intervl
                    )
            );
        }

        /**
         * \brief Sets the interval for the task execution using a duration in nanoseconds.
         *
         * This function sets the interval for the task execution using a duration specified
         * in nanoseconds. The interval determines how often the task will be executed after
         * the initial execution.
         *
         * \param intervl The interval duration in nanoseconds.
         */
        inline void setIntervalChronoNanos(std::chrono::nanoseconds intervl)
        {
            mNanoInterval = intervl;
            mHasSetInterval = true;
            OnIntervalChanged();
        }

        /**
         * \brief Updates the task execution.
         *
         * This function updates the task execution. It checks if the task is ready to be
         * executed based on the set interval, and if so, it executes the task function.
         * If the task is not ready to be executed, the function returns without performing
         * any action.
         */
        inline void Update() {
            if (CanExecuteTask() == false)
                return;

            mTask();
        }

    private:


        /**
         * \brief Handles the interval change event.
         *
         * This function is called when the interval of the task is changed. It updates
         * the next execution timestamp based on the current timestamp and the new interval.
         */
        inline void OnIntervalChanged()
        {
            mNextExecStamp = CurrNanoTimeStamp() + mNanoInterval;
        }

        /**
         * \brief Checks if the task can be executed.
         *
         * This function determines whether the task is eligible for execution based on the
         * interval and current timestamp. If the interval has been set and the next execution
         * timestamp has passed, the task can be executed.
         *
         * \return `true` if the task can be executed, `false` otherwise.
         */
        inline bool CanExecuteTask()
        {
            if (mHasSetInterval == false)
                return false;

            auto currTime = CurrNanoTimeStamp();

            if (mNextExecStamp > currTime)
                return false;

            mNextExecStamp = currTime + mNanoInterval;

            return true;
        }

        bool mHasSetInterval;
        std::function<void()> mTask;
        std::chrono::nanoseconds mNextExecStamp;
        std::chrono::nanoseconds mNanoInterval;

    };

    class TaskManager {
    public:

        /**
         * \brief Adds a task to the task manager with a specified unique ID.
         *
         * This function adds a task to the task manager using the provided unique ID. The task
         * is passed as an rvalue reference to a unique pointer. The function takes ownership of
         * the task and stores it in the task manager.
         *
         * \param uid The unique ID for the task.
         * \param _tsk The task to be added (as an rvalue reference to a unique pointer).
         */
        inline void Add(const std::string& uid, std::unique_ptr<Task>&& _tsk)
        {
            std::unique_ptr<Task>& tsk = _tsk;

            Add(uid, tsk);
        }

        /**
         * \brief Adds a task to the task manager.
         *
         * This function adds a task to the task manager. The task is passed as an rvalue reference to
         * a unique pointer. The function takes ownership of the task and stores it in the task manager.
         *
         * \param _tsk The task to be added (as an rvalue reference to a unique pointer).
         */
        inline void Add(std::unique_ptr<Task>&& _tsk) {
            std::unique_ptr<Task>& tsk = _tsk;

            Add(tsk);
        }

        /**
        * \brief Adds a task to the task manager with an auto-generated unique ID.
        *
        * This function adds a task to the task manager using an auto-generated unique ID. The task
        * is passed as a reference to a unique pointer. The function takes ownership of the task
        * and stores it in the task manager.
        *
        * \param tsk The task to be added (as a reference to a unique pointer).
        */
        inline void Add(std::unique_ptr<Task>& tsk)
        {
            Add(std::to_string((unsigned long long)tsk.get()), tsk);
        }

        /**
         * \brief Adds a task to the task manager with a specified UID.
         *
         * This function adds a task to the task manager with the specified UID. If a task with the same UID
         * already exists in the task manager, the function does nothing.
         *
         * \param uid The UID of the task.
         * \param tsk A unique pointer to the task to be added.
         */
        inline void Add(const std::string& uid, std::unique_ptr<Task>& tsk)
        {
            if (mAllTasks.find(uid) != mAllTasks.end())
                return; // Task with Same UID Alredy Exist

            mAllTasks[uid] = std::move(tsk);
        }

        /**
         * \brief Removes a task with the specified UID from the task manager.
         *
         * This function removes a task with the specified UID from the task manager. If no task with the specified UID
         * is found in the task manager, the function does nothing.
         *
         * \param uid The UID of the task to be removed.
         */
        inline void Remove(const std::string& uid)
        {
            if (mAllTasks.find(uid) == mAllTasks.end())
                return; // Task not found

            mAllTasks.erase(uid);
        }

        /**
         * \brief Updates all tasks in the task manager.
         *
         * This function updates all tasks in the task manager by calling the `Update` function of each task.
         * It iterates over all tasks in the task manager and invokes their `Update` function to perform any
         * necessary updates.
         *
         * \remarks The `Update` function of each task will be called in the order they were added to the task manager.
         */
        inline void Update()
        {
            for (const auto& curr : mAllTasks)
                curr.second->Update();
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<Task>> mAllTasks;
    };
}