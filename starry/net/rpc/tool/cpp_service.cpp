#include "cpp_service.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <string_view>

namespace starry {
namespace compiler {

// 定义全局常量字符串，这些在gen.cpp中也会使用
const char kThickSeparator[] =
    "// ===================================================================\n";
const char kThinSeparator[] =
    "// -------------------------------------------------------------------\n";

ServiceGenerator::ServiceGenerator(
    const google::protobuf::ServiceDescriptor* descriptor,
    std::string_view filename,
    int index)
    : descriptor_(descriptor) {
  vars_["classname"] = descriptor_->name();
  vars_["full_name"] = descriptor_->full_name();
  vars_["filename"] = std::string(filename);
  vars_["index"] = std::to_string(index);
}

ServiceGenerator::~ServiceGenerator() = default;

void ServiceGenerator::GenerateDeclarations(
    google::protobuf::io::Printer* printer) {
  // 前向声明存根类型
  printer->Print(vars_,
                 "class $classname$_Stub;\n"
                 "\n");

  GenerateInterface(printer);
  GenerateStubDefinition(printer);
}

void ServiceGenerator::GenerateInterface(
    google::protobuf::io::Printer* printer) {
  printer->Print(vars_,
                 "class $classname$ : public ::google::protobuf::Service {\n"
                 " protected:\n"
                 "  // 这个类应该被视为抽象接口\n"
                 "  $classname$() = default;\n"
                 " public:\n"
                 "  virtual ~$classname$();\n");
  printer->Indent();

  printer->Print(
      vars_,
      "\n"
      "using Stub = $classname$_Stub;\n"
      "\n"
      "static const ::google::protobuf::ServiceDescriptor* descriptor();\n"
      "\n");

  GenerateMethodSignatures(NON_STUB, printer);

  printer->Print(
      "\n"
      "// 实现Service接口 ----------------------------------------------\n"
      "\n"
      "const ::google::protobuf::ServiceDescriptor* GetDescriptor() override;\n"
      "void CallMethod(const ::google::protobuf::MethodDescriptor* method,\n"
      "                ::google::protobuf::RpcController* controller,\n"
      "                const ::google::protobuf::Message* request,\n"
      "                ::google::protobuf::Message* response,\n"
      "                ::google::protobuf::Closure* done) override;\n"
      "const ::google::protobuf::Message& GetRequestPrototype(\n"
      "  const ::google::protobuf::MethodDescriptor* method) const override;\n"
      "const ::google::protobuf::Message& GetResponsePrototype(\n"
      "  const ::google::protobuf::MethodDescriptor* method) const "
      "override;\n");

  printer->Outdent();
  printer->Print(vars_,
                 "\n"
                 " private:\n"
                 "  $classname$(const $classname$&) = delete;\n"
                 "  $classname$& operator=(const $classname$&) = delete;\n"
                 "};\n"
                 "\n");
}

void ServiceGenerator::GenerateStubDefinition(
    google::protobuf::io::Printer* printer) {
  printer->Print(vars_,
                 "class $classname$_Stub : public $classname$ {\n"
                 " public:\n");

  printer->Indent();

  printer->Print(
      vars_,
      "explicit $classname$_Stub(::google::protobuf::RpcChannel* channel);\n"
      "~$classname$_Stub() override;\n"
      "\n"
      "[[nodiscard]] ::google::protobuf::RpcChannel* channel() const { return "
      "channel_; }\n"
      "\n"
      "// 实现 $classname$ 接口 ------------------------------------------\n"
      "\n");

  GenerateMethodSignatures(STUB, printer);

  printer->Outdent();
  printer->Print(
      vars_,
      " private:\n"
      "  ::google::protobuf::RpcChannel* channel_;\n"
      "  $classname$_Stub(const $classname$_Stub&) = delete;\n"
      "  $classname$_Stub& operator=(const $classname$_Stub&) = delete;\n"
      "};\n"
      "\n");
}

void ServiceGenerator::GenerateMethodSignatures(
    StubOrNon stub_or_non,
    google::protobuf::io::Printer* printer) {
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::map<std::string, std::string> sub_vars;
    sub_vars["classname"] = descriptor_->name();
    sub_vars["name"] = method->name();
    sub_vars["input_type"] = ClassName(method->input_type(), true);
    sub_vars["output_type"] = ClassName(method->output_type(), true);
    sub_vars["virtual"] = "virtual ";

    if (stub_or_non == NON_STUB) {
      printer->Print(
          sub_vars,
          "$virtual$void $name$(::google::protobuf::RpcController* "
          "controller,\n"
          "                     const $input_type$* request,\n"
          "                     $output_type$* response,\n"
          "                     ::google::protobuf::Closure* done) = 0;\n");
    } else {
      printer->Print(sub_vars,
                     "using $classname$::$name$;\n"
                     "$virtual$void $name$(::google::protobuf::RpcController* "
                     "controller,\n"
                     "                     const $input_type$* request,\n"
                     "                     $output_type$* response,\n"
                     "                     ::google::protobuf::Closure* done) "
                     "override;\n");
    }
  }
}

void ServiceGenerator::GenerateDescriptorInitializer(
    google::protobuf::io::Printer* printer,
    int index) {
  std::map<std::string, std::string> vars;
  vars["classname"] = descriptor_->name();
  vars["index"] = std::to_string(index);

  printer->Print(vars, "$classname$_descriptor_ = file->service($index$);\n");
}

void ServiceGenerator::GenerateImplementation(
    google::protobuf::io::Printer* printer) {
  printer->Print(
      vars_,
      "$classname$::~$classname$() = default;\n"
      "\n"
      "namespace {\n"
      "inline const ::google::protobuf::ServiceDescriptor* "
      "GetServiceDescriptor() {\n"
      "  static const ::google::protobuf::ServiceDescriptor* descriptor = "
      "nullptr;\n"
      "  if (!descriptor) {\n"
      "    descriptor = "
      "::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(\n"
      "        \"$filename$\")->service($index$);\n"
      "  }\n"
      "  return descriptor;\n"
      "}\n"
      "} // namespace\n"
      "\n"
      "const ::google::protobuf::ServiceDescriptor* $classname$::descriptor() "
      "{\n"
      "  return GetServiceDescriptor();\n"
      "}\n"
      "\n"
      "const ::google::protobuf::ServiceDescriptor* "
      "$classname$::GetDescriptor() {\n"
      "  return descriptor();\n"
      "}\n"
      "\n");

  // 生成接口的方法
  GenerateCallMethod(printer);
  GenerateGetPrototype(REQUEST, printer);
  GenerateGetPrototype(RESPONSE, printer);

  // 生成存根实现
  printer->Print(vars_,
                 "$classname$_Stub::$classname$_Stub(::google::protobuf::"
                 "RpcChannel* channel)\n"
                 "  : channel_(channel) {}\n"
                 "$classname$_Stub::~$classname$_Stub() = default;\n"
                 "\n");

  GenerateStubMethods(printer);
}

void ServiceGenerator::GenerateNotImplementedMethods(
    google::protobuf::io::Printer* printer) {
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::map<std::string, std::string> sub_vars;
    sub_vars["classname"] = descriptor_->name();
    sub_vars["name"] = method->name();
    sub_vars["index"] = std::to_string(i);
    sub_vars["input_type"] = ClassName(method->input_type(), true);
    sub_vars["output_type"] = ClassName(method->output_type(), true);

    printer->Print(
        sub_vars,
        "void $classname$::$name$(::google::protobuf::RpcController* "
        "controller,\n"
        "                         const $input_type$* request,\n"
        "                         $output_type$* response,\n"
        "                         ::google::protobuf::Closure* done) {\n"
        "  controller->SetFailed(\"Method $name$() not implemented.\");\n"
        "  done->Run();\n"
        "}\n"
        "\n");
  }
}

void ServiceGenerator::GenerateCallMethod(
    google::protobuf::io::Printer* printer) {
  printer->Print(
      vars_,
      "void $classname$::CallMethod(const "
      "::google::protobuf::MethodDescriptor* method,\n"
      "                           ::google::protobuf::RpcController* "
      "controller,\n"
      "                           const ::google::protobuf::Message* request,\n"
      "                           ::google::protobuf::Message* response,\n"
      "                           ::google::protobuf::Closure* done) {\n"
      "  assert(method->service() == GetServiceDescriptor());\n"
      "  switch(method->index()) {\n");

  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::map<std::string, std::string> sub_vars;
    sub_vars["name"] = method->name();
    sub_vars["index"] = std::to_string(i);
    sub_vars["input_type"] = ClassName(method->input_type(), true);
    sub_vars["output_type"] = ClassName(method->output_type(), true);

    printer->Print(sub_vars,
                   "    case $index$:\n"
                   "      $name$(controller,\n"
                   "             down_cast<const $input_type$*>(request),\n"
                   "             down_cast<$output_type$*>(response),\n"
                   "             done);\n"
                   "      break;\n");
  }

  printer->Print(vars_,
                 "    default:\n"
                 "      assert(false && \"Bad method index\");\n"
                 "      break;\n"
                 "  }\n"
                 "}\n"
                 "\n");
}

void ServiceGenerator::GenerateGetPrototype(
    RequestOrResponse which,
    google::protobuf::io::Printer* printer) {
  if (which == REQUEST) {
    printer->Print(vars_,
                   "const ::google::protobuf::Message& "
                   "$classname$::GetRequestPrototype(\n");
  } else {
    printer->Print(vars_,
                   "const ::google::protobuf::Message& "
                   "$classname$::GetResponsePrototype(\n");
  }

  printer->Print(
      vars_,
      "    const ::google::protobuf::MethodDescriptor* method) const {\n"
      "  assert(method->service() == descriptor());\n"
      "  switch(method->index()) {\n");

  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    const google::protobuf::Descriptor* type =
        (which == REQUEST) ? method->input_type() : method->output_type();

    std::map<std::string, std::string> sub_vars;
    sub_vars["index"] = std::to_string(i);
    sub_vars["type"] = ClassName(type, true);

    printer->Print(sub_vars,
                   "    case $index$:\n"
                   "      return $type$::default_instance();\n");
  }

  printer->Print(vars_,
                 "    default:\n"
                 "      assert(false && \"Bad method index\");\n"
                 "      abort();\n"
                 "  }\n"
                 "}\n"
                 "\n");
}

void ServiceGenerator::GenerateStubMethods(
    google::protobuf::io::Printer* printer) {
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::map<std::string, std::string> sub_vars;
    sub_vars["classname"] = descriptor_->name();
    sub_vars["name"] = method->name();
    sub_vars["index"] = std::to_string(i);
    sub_vars["input_type"] = ClassName(method->input_type(), true);
    sub_vars["output_type"] = ClassName(method->output_type(), true);

    printer->Print(
        sub_vars,
        "void $classname$_Stub::$name$(::google::protobuf::RpcController* "
        "controller,\n"
        "                            const $input_type$* request,\n"
        "                            $output_type$* response,\n"
        "                            ::google::protobuf::Closure* done) {\n"
        "  channel_->CallMethod(descriptor()->method($index$),\n"
        "                     controller, request, response, done);\n"
        "}\n");
  }
}

// std::string ClassName(const google::protobuf::Descriptor* descriptor,
//                       bool qualified) {
//   // 在本函数中实现获取类名的逻辑，根据是否需要限定名
//   std::string result;
//   if (qualified) {
//     result = descriptor->file()->package();
//     if (!result.empty()) {
//       result += "::";
//     }
//   }
//
//   if (descriptor->containing_type() != nullptr) {
//     result += ClassName(descriptor->containing_type(), false) + "_";
//   }
//
//   result += descriptor->name();
//   return result;
// }
//
// std::string StripProto(const std::string& filename) {
//   if (filename.length() >= 6 &&
//       filename.compare(filename.length() - 6, 6, ".proto") == 0) {
//     return filename.substr(0, filename.length() - 6);
//   }
//   return filename;
// }

}  // namespace compiler
}  // namespace starry
