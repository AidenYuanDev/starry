#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <map>
#include <string>
#include <string_view>

namespace starry::compiler {

extern const std::string kThickSeparator;
extern const std::string kThinSeparator;

class ServiceGenerator {
 public:
  enum StubOrNon { NON_STUB, STUB };
  enum RequestOrResponse { REQUEST, RESPONSE };
  ServiceGenerator(const google::protobuf::ServiceDescriptor* descriptor,
                   std::string_view filename,
                   int index);
  ~ServiceGenerator() = default;
  ServiceGenerator(const ServiceGenerator&) = delete;
  ServiceGenerator& operator=(const ServiceGenerator&) = delete;

  void generateDeclarations(google::protobuf::io::Printer* printer);
  void generateImplementation(google::protobuf::io::Printer* printer);

  static void genHeader(google::protobuf::io::Printer& printer);

 private:
  void generateInterface(google::protobuf::io::Printer* printer);
  void generateStubDefinition(google::protobuf::io::Printer* printer);
  void generateMethodSignatures(StubOrNon stub_or_non,
                                google::protobuf::io::Printer* printer);
  void generateDescriptorInitializer(google::protobuf::io::Printer* printer,
                                     int index);
  void generateNotImplementedMethods(google::protobuf::io::Printer* printer);
  void generateCallMethod(google::protobuf::io::Printer* printer);
  void generateGetPrototype(RequestOrResponse which,
                            google::protobuf::io::Printer* printer);
  void generateStubMethods(google::protobuf::io::Printer* printer);

  const google::protobuf::ServiceDescriptor* descriptor_;
  std::map<std::string, std::string> vars_;
};

inline std::string ClassName(const google::protobuf::Descriptor* descriptor,
                             bool qualified) {
  std::string result;
  if (qualified) {
    result = descriptor->file()->package();
    if (!result.empty()) {
      result += "::";
    }
  }

  if (descriptor->containing_type() != nullptr) {
    result += ClassName(descriptor->containing_type(), false) + "_";
  }

  result += descriptor->name();
  return result;
}

inline std::string protoBaseName(const std::string& filename) {
  std::size_t pos = filename.find_last_of(".");
  assert(pos != std::string::npos);

  return filename.substr(0, pos);
}
}  // namespace starry::compiler
