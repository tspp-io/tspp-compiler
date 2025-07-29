#pragma once
#include <memory>
#include <vector>

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class Expression : public ASTNode {
};

class Statement : public ASTNode {
};
