#pragma once

#define ACCEPT_VISITOR()                      \
  void accept(ASTVisitor& visitor) override { \
    visitor.visit(*this);                     \
  }

#define NODEPTR(T) std::shared_ptr<T>
#define NODELIST(T) std::vector<std::shared_ptr<T>>
