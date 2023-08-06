#pragma once

/*
* Описание: предоставить интерфейс для правильного создания файла JSON.
 */
#include <memory>
#include <optional>
#include <queue>

#include "json.h"

namespace json {

class Builder;
class DictContext;
class ArrayContext;
class ValueContext;

/* BASE CONTEXT */

/*
* Контексты ниже предоставляют интерфейсы для правильного создания JSON.
 * Например, они не позволяют создавать JSON: {"name", [ } и многие другие.
 * Идея: создать контекст + добавить в него только нужные методы, чтобы при вызове неправильного метода пользователь получал
 * compilation error.
 */

class BaseContext {
public:  
    explicit BaseContext(Builder& builder);

protected:  
    Builder& builder_;
};

class StartContainersContext : public BaseContext {
public:   
    explicit StartContainersContext(Builder& builder);

public:   
    ArrayContext& StartArray();
    DictContext& StartDict();
};

/* JSON CONTEXTS */

class KeyContext : public StartContainersContext {
   
public:   
    explicit KeyContext(Builder& builder);

public:   
    ValueContext Value(Node::Value value);
};

class ValueContext : public BaseContext {
    
public:  
    explicit ValueContext(Builder& builder);

public:   
    KeyContext& Key(std::string key);
    Builder& EndDict();
};

class DictContext : public BaseContext {
 
public:   
    explicit DictContext(Builder& builder);

public:   
    KeyContext& Key(std::string key);
    Builder& EndDict();
};

class ArrayContext : public StartContainersContext {
    /* Methods: Value, EndArray | StartDict, StartArray */
public:  
    explicit ArrayContext(Builder& builder);

public:  
    ArrayContext& Value(Node::Value value);
    Builder& EndArray();
};

/* BUILDER */

/// Предоставляет интерфейс для правильного создания JSON.
/// @выдает std::logical_error в случае некорректной попытки создания JSON
class Builder final : virtual public KeyContext,
                      virtual public ValueContext,
                      virtual public DictContext,
                      virtual public ArrayContext {
public:   
    Builder();

public:   // методы
    KeyContext& Key(std::string key);
    Builder& Value(Node::Value value);

    DictContext& StartDict();
    Builder& EndDict();

    ArrayContext& StartArray();
    Builder& EndArray();

    Node& Build();

private:  // методы
    [[nodiscard]] bool CouldAddNode() const;

    void AddNode(Node top_node);   
                          
private: 
    Node root_{nullptr};
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};
}  // namespace json
