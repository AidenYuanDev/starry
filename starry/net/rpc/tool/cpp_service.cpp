#include "cpp_service.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <string>
#include <string_view>

namespace starry::compiler {

const std::string kThickSeparator =
    "// ===================================================================\n";
const std::string kThinSeparator =
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

void ServiceGenerator::generateDeclarations(
    google::protobuf::io::Printer* printer) {
  // 前向声明存根类型
  printer->Print(vars_,
                 "class $classname$_Stub;\n"
                 "\n");

  generateInterface(printer);
  generateStubDefinition(printer);
}

void ServiceGenerator::generateInterface(
    google::protobuf::io::Printer* printer) {
  printer->Print(vars_,
                 "class $classname$ : public ::starry::Service {\n"
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

  generateMethodSignatures(NON_STUB, printer);

  printer->Print(
      "\n"
      "// 实现Service接口 ----------------------------------------------\n"
      "\n"
      "const ::google::protobuf::ServiceDescriptor* GetDescriptor() override;\n"
      "void CallMethod(const ::google::protobuf::MethodDescriptor* method,\n"
      "                const ::google::protobuf::MessagePtr&* request,\n"
      "                ::google::protobuf::Message* responsePrototype,\n"
      "                const ::starry::RpcDoneCallback& done) override;\n"
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

void ServiceGenerator::generateStubDefinition(
    google::protobuf::io::Printer* printer) {
  printer->Print(vars_,
                 "class $classname$_Stub : public $classname$ {\n"
                 " public:\n");

  printer->Indent();

  printer->Print(
      vars_,
      "explicit $classname$_Stub(::starry::RpcChannel* channel);\n"
      "~$classname$_Stub() override;\n"
      "\n"
      "inline ::starry::RpcChannel* channel() { return channel_; }\n"
      "\n"
      "// 实现 $classname$ 接口 ------------------------------------------\n"
      "\n");

  generateMethodSignatures(STUB, printer);

  printer->Outdent();
  printer->Print(
      vars_,
      " private:\n"
      "  ::starry::RpcChannel* channel_;\n"
      "  bool owns_channel_;\n"
      "  $classname$_Stub(const $classname$_Stub&) = delete;\n"
      "  $classname$_Stub& operator=(const $classname$_Stub&) = delete;\n"
      "};\n"
      "\n");
}

void ServiceGenerator::generateMethodSignatures(
    StubOrNon stub_or_non,
    google::protobuf::io::Printer* printer) {
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::map<std::string, std::string> sub_vars;
    sub_vars["classname"] = descriptor_->name();
    sub_vars["name"] = method->name();
    sub_vars["input_type"] = ClassName(method->input_type(), true);
    sub_vars["output_type"] = ClassName(method->output_type(), true);
    sub_vars["output_typedef"] = ClassName(method->output_type(), true);
    sub_vars["virtual"] = "virtual ";

    if (stub_or_non == NON_STUB) {
      printer->Print(
          sub_vars,
          "$virtual$void $name$(const $input_type$Ptr& request,\n"
          "                     const $output_type$* responsePrototype,\n"
          "                     const ::starry::RpcDoneCallback& done);\n");
    } else {
      printer->Print(sub_vars,
                     "using $classname$::$name$;\n"
                     "$virtual$void $name$(const $input_type$& request,\n"
                     "                     const ::std::function<void(const "
                     "$output_typedef$Ptr&)>& done);\n");
    }
  }
}

void ServiceGenerator::generateDescriptorInitializer(
    google::protobuf::io::Printer* printer,
    int index) {
  std::map<std::string, std::string> vars;
  vars["classname"] = descriptor_->name();
  vars["index"] = std::to_string(index);

  printer->Print(vars, "$classname$_descriptor_ = file->service($index$);\n");
}

void ServiceGenerator::generateImplementation(
    google::protobuf::io::Printer* printer) {
  printer->Print(
      vars_,
      "$classname$::~$classname$() {}\n"
      "\n"
      "static const ::google::protobuf::ServiceDescriptor* "
      "$classname$_descriptor_ = NULL;\n"
      "\n"
      "const ::google::protobuf::ServiceDescriptor* $classname$::descriptor() "
      "{\n"
      "  protobuf_AssignDescriptorsOnce();\n"
      "  if ($classname$_descriptor_ == NULL)\n"
      "    $classname$_descriptor_ = "
      "::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(\n"
      "        \"$filename$\")->service($index$);\n"
      "  return $classname$_descriptor_;\n"
      "}\n"
      "\n"
      "const ::google::protobuf::ServiceDescriptor* "
      "$classname$::GetDescriptor() {\n"
      "  return descriptor();\n"
      "}\n"
      "\n");

  // 生成接口的方法
  generateNotImplementedMethods(printer);
  generateCallMethod(printer);
  generateGetPrototype(REQUEST, printer);
  generateGetPrototype(RESPONSE, printer);

  // 生成存根实现
  printer->Print(
      vars_,
      "$classname$_Stub::$classname$_Stub(::starry::RpcChannel* channel)\n"
      "  : channel_(channel), owns_channel_(false) {}\n"
      "\n"
      "$classname$_Stub::~$classname$_Stub() {\n"
      "  if (owns_channel_ && channel_) {\n"
      "    delete channel_;\n"
      "    channel_ = nullptr;\n"
      "  }\n"
      "}\n"
      "\n");
  generateStubMethods(printer);
}

void ServiceGenerator::generateNotImplementedMethods(
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
        "void $classname$::$name$(const $input_type$Ptr&,\n"
        "                         const $output_type$*,\n"
        "                         const ::starry::RpcDoneCallback& done) "
        "{\n"
        // "  controller->SetFailed(\"Method $name$() not implemented.\");\n"
        "  assert(0);\n"
        "  done(NULL);\n"
        "}\n"
        "\n");
  }
}

void ServiceGenerator::generateCallMethod(
    google::protobuf::io::Printer* printer) {
  printer->Print(
      vars_,
      "void $classname$::CallMethod(const "
      "::google::protobuf::MethodDescriptor* method,\n"
      "                             const ::google::protobuf::MessagePtr& "
      "request,\n"
      "                             const ::google::protobuf::Message* "
      "responsePrototype,\n"
      "                             const ::starry::RpcDoneCallback& done) {\n"
      "  GOOGLE_DCHECK_EQ(method->service(), $classname$_descriptor_);\n"
      "  switch(method->index()) {\n");

  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::map<std::string, std::string> sub_vars;
    sub_vars["name"] = method->name();
    sub_vars["index"] = std::to_string(i);
    sub_vars["input_type"] = ClassName(method->input_type(), true);
    sub_vars["output_type"] = ClassName(method->output_type(), true);

    printer->Print(
        sub_vars,
        "    case $index$:\n"
        "      $name$(std::static_pointer_cast<const $input_type$>(request),\n"
        "             down_cast<$output_type$*>(responsePrototype),\n"
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

void ServiceGenerator::generateGetPrototype(
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

void ServiceGenerator::generateStubMethods(
    google::protobuf::io::Printer* printer) {
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::map<std::string, std::string> sub_vars;
    sub_vars["classname"] = descriptor_->name();
    sub_vars["name"] = method->name();
    sub_vars["index"] = std::to_string(i);
    sub_vars["input_type"] = ClassName(method->input_type(), true);
    sub_vars["output_type"] = ClassName(method->output_type(), true);
    sub_vars["output_typedef"] = ClassName(method->output_type(), true);

    printer->Print(
        sub_vars,
        "void $classname$_Stub::$name$(const $input_type$& request,\n"
        "                              const ::std::function<void(const "
        "$output_typedef$Ptr&)>& done) {\n"
        "  channel_->CallMethod(descriptor()->method($index$),\n"
        "                       request, &$output_type$::default_instance(), "
        "done);\n"
        "}\n");
  }
}

void ServiceGenerator::genHeader(google::protobuf::io::Printer& printer) {
  printer.Print("#include \"service.h\"\n");
  printer.Print("#include <memory.h>\n");
}

}  // namespace starry::compiler
