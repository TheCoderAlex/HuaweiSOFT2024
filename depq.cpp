/*
 * @Author: thecoderalex tangzf2001@126.com
 * @Date: 2024-03-18 10:52:26
 * @LastEditors: thecoderalex tangzf2001@126.com
 * @LastEditTime: 2024-03-18 11:06:06
 * @FilePath: \HuaweiSOFT\depq.cpp
 * @Description: 使用红黑树实现的双端优先队列
 * 
 * Copyright (c) 2024 by 菜就多练, All Rights Reserved. 
 */
#include <set>
#include <iostream>

template<typename T, typename Comparator = std::less<T>>
class DequePriorityQueue {
private:
    std::set<T, Comparator> data;
public:
    void insert(const T& item) {
        data.insert(item);
    }

    T getMin() const {
        return *data.begin();
    }

    void removeMin() {
        if (!data.empty()) {
            data.erase(data.begin());
        }
    }

    T getMax() const {
        return *data.rbegin();
    }

    void removeMax() {
        if (!data.empty()) {
            auto it = data.end();
            --it;
            data.erase(it);
        }
    }

    unsigned int size() {
        if (data.empty()) {
            return (unsigned) 0;
        } else
            return data.size();
    }
    
    bool isEmpty() const {
        return data.empty();
    }
};