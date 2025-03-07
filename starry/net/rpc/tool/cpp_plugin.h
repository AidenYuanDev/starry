#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <string>

namespace google::protobuf {

class FileDescriptor;

}

namespace starry::compiler {

class CppStarryGenerator : public google::protobuf::compiler::CodeGenerator {
 public:
  CppStarryGenerator() {}

  virtual ~CppStarryGenerator() {}

  bool Generate(const google::protobuf::FileDescriptor* file,
                const std::string& parameter,
                google::protobuf::compiler::GeneratorContext* context,
                std::string* error) const;
};

}
