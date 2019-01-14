#include "AccessFlag.h"
#include "ClassFile.h"
#include "Debug.h"
#include "Interpreter.hpp"
#include "JavaClass.h"
#include "JavaHeap.hpp"
#include "Option.h"

#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>

#define IS_COMPUTATIONAL_TYPE_1(value) \
    (typeid(*value) != typeid(JDouble) && typeid(*value) != typeid(JLong))
#define IS_COMPUTATIONAL_TYPE_2(value) \
    (typeid(*value) == typeid(JDouble) || typeid(*value) == typeid(JLong))

#pragma warning(disable : 4715)
#pragma warning(disable : 4244)

JType *Interpreter::execCode(const JavaClass *jc, CodeAttrCore &&ext) {
    for (decltype(ext.codeLength) op = 0; op < ext.codeLength; op++) {
        // If callee propagates a unhandled exception, try to handle  it. When
        // we can not handle it, propagates it to upper and returns
        if (exception.hasUnhandledException()) {
            op--;
            auto *throwableObject = currentStackPop<JObject>();
            if (throwableObject == nullptr) {
                throw std::runtime_error("null pointer");
            }
            if (!hasInheritanceRelationship(
                    throwableObject->jc,
                    yrt.ma->loadClassIfAbsent("java/lang/Throwable"))) {
                throw std::runtime_error("it's not a throwable object");
            }

            if (handleException(jc, ext, throwableObject, op)) {
                while (!currentFrame->stack.empty()) {
                    auto *temp = currentStackPop<JType>();
                    delete temp;
                }
                currentFrame->stack.push_back(throwableObject);
                exception.sweepException();
            } else {
                return throwableObject;
            }
        }
#ifdef YVM_DEBUG_SHOW_BYTECODE
        for (int i = 0; i < frames.size(); i++) {
            std::cout << "-";
        }
        Inspector::printOpcode(ext.code, op);
#endif
        // Interpreting through big switching
        switch (ext.code[op]) {
            case op_nop: {
                // DO NOTHING :-)
            } break;
            case op_aconst_null: {
                JObject *obj = nullptr;
                currentFrame->stack.push_back(obj);
            } break;
            case op_iconst_m1: {
                currentFrame->stack.push_back(new JInt(-1));
            } break;
            case op_iconst_0: {
                currentFrame->stack.push_back(new JInt(0));
            } break;
            case op_iconst_1: {
                currentFrame->stack.push_back(new JInt(1));
            } break;
            case op_iconst_2: {
                currentFrame->stack.push_back(new JInt(2));
            } break;
            case op_iconst_3: {
                currentFrame->stack.push_back(new JInt(3));
            } break;
            case op_iconst_4: {
                currentFrame->stack.push_back(new JInt(4));
            } break;
            case op_iconst_5: {
                currentFrame->stack.push_back(new JInt(5));
            } break;
            case op_lconst_0: {
                currentFrame->stack.push_back(new JLong(0));
            } break;
            case op_lconst_1: {
                currentFrame->stack.push_back(new JLong(1));
            } break;
            case op_fconst_0: {
                currentFrame->stack.push_back(new JFloat(0.0f));
            } break;
            case op_fconst_1: {
                currentFrame->stack.push_back(new JFloat(1.0f));
            } break;
            case op_fconst_2: {
                currentFrame->stack.push_back(new JFloat(2.0f));
            } break;
            case op_dconst_0: {
                currentFrame->stack.push_back(new JDouble(0.0));
            } break;
            case op_dconst_1: {
                currentFrame->stack.push_back(new JDouble(1.0));
            } break;
            case op_bipush: {
                const u1 byte = consumeU1(ext.code, op);
                currentFrame->stack.push_back(new JInt(byte));
            } break;
            case op_sipush: {
                const u2 byte = consumeU2(ext.code, op);
                currentFrame->stack.push_back(new JInt(byte));
            } break;
            case op_ldc: {
                const u1 index = consumeU1(ext.code, op);
                loadConstantPoolItem2Stack(jc, static_cast<u2>(index));
            } break;
            case op_ldc_w: {
                const u2 index = consumeU2(ext.code, op);
                loadConstantPoolItem2Stack(jc, index);
            } break;
            case op_ldc2_w: {
                const u2 index = consumeU2(ext.code, op);
                if (typeid(*jc->raw.constPoolInfo[index]) ==
                    typeid(CONSTANT_Double)) {
                    auto val = dynamic_cast<CONSTANT_Double *>(
                                   jc->raw.constPoolInfo[index])
                                   ->val;
                    JDouble *dval = new JDouble;
                    dval->val = val;
                    currentFrame->stack.push_back(dval);
                } else if (typeid(*jc->raw.constPoolInfo[index]) ==
                           typeid(CONSTANT_Long)) {
                    auto val = dynamic_cast<CONSTANT_Long *>(
                                   jc->raw.constPoolInfo[index])
                                   ->val;
                    JLong *lval = new JLong;
                    lval->val = val;
                    currentFrame->stack.push_back(lval);
                } else {
                    throw std::runtime_error(
                        "invalid symbolic reference index on "
                        "constant pool");
                }
            } break;
            case op_iload: {
                const u1 index = consumeU1(ext.code, op);
                load2Stack<JInt>(index);
            } break;
            case op_lload: {
                const u1 index = consumeU1(ext.code, op);
                load2Stack<JLong>(index);
            } break;
            case op_fload: {
                const u1 index = consumeU1(ext.code, op);
                load2Stack<JFloat>(index);
            } break;
            case op_dload: {
                const u1 index = consumeU1(ext.code, op);
                load2Stack<JDouble>(index);
            } break;
            case op_aload: {
                const u1 index = consumeU1(ext.code, op);
                load2Stack<JRef>(index);
            } break;
            case op_iload_0: {
                load2Stack<JInt>(0);
            } break;
            case op_iload_1: {
                load2Stack<JInt>(1);
            } break;
            case op_iload_2: {
                load2Stack<JInt>(2);
            } break;
            case op_iload_3: {
                load2Stack<JInt>(3);
            } break;
            case op_lload_0: {
                load2Stack<JLong>(0);
            } break;
            case op_lload_1: {
                load2Stack<JLong>(1);
            } break;
            case op_lload_2: {
                load2Stack<JLong>(2);
            } break;
            case op_lload_3: {
                load2Stack<JLong>(3);
            } break;
            case op_fload_0: {
                load2Stack<JFloat>(0);
            } break;
            case op_fload_1: {
                load2Stack<JFloat>(1);
            } break;
            case op_fload_2: {
                load2Stack<JFloat>(2);
            } break;
            case op_fload_3: {
                load2Stack<JFloat>(3);
            } break;
            case op_dload_0: {
                load2Stack<JDouble>(0);
            } break;
            case op_dload_1: {
                load2Stack<JDouble>(1);
            } break;
            case op_dload_2: {
                load2Stack<JDouble>(2);
            } break;
            case op_dload_3: {
                load2Stack<JDouble>(3);
            } break;
            case op_aload_0: {
                load2Stack<JRef>(0);
            } break;
            case op_aload_1: {
                load2Stack<JRef>(1);
            } break;
            case op_aload_2: {
                load2Stack<JRef>(2);
            } break;
            case op_aload_3: {
                load2Stack<JRef>(3);
            } break;
            case op_saload:
            case op_caload:
            case op_baload:
            case op_iaload: {
                loadArrayItem2Stack<JInt>();
            } break;
            case op_laload: {
                loadArrayItem2Stack<JLong>();
            } break;
            case op_faload: {
                loadArrayItem2Stack<JFloat>();
            } break;
            case op_daload: {
                loadArrayItem2Stack<JDouble>();
            } break;
            case op_aaload: {
                loadArrayItem2Stack<JRef>();
            } break;
            case op_istore: {
                const u1 index = consumeU1(ext.code, op);
                store2Local<JInt>(index);
            } break;
            case op_lstore: {
                const u1 index = consumeU1(ext.code, op);
                store2Local<JLong>(index);
            } break;
            case op_fstore: {
                const u1 index = consumeU1(ext.code, op);
                store2Local<JFloat>(index);
            } break;
            case op_dstore: {
                const u1 index = consumeU1(ext.code, op);
                store2Local<JDouble>(index);
            } break;
            case op_astore: {
                const u1 index = consumeU1(ext.code, op);
                store2Local<JRef>(index);
            } break;
            case op_istore_0: {
                store2Local<JInt>(0);
            } break;
            case op_istore_1: {
                store2Local<JInt>(1);
            } break;
            case op_istore_2: {
                store2Local<JInt>(2);
            } break;
            case op_istore_3: {
                store2Local<JInt>(3);
            } break;
            case op_lstore_0: {
                store2Local<JLong>(0);
            } break;
            case op_lstore_1: {
                store2Local<JLong>(1);
            } break;
            case op_lstore_2: {
                store2Local<JLong>(2);
            } break;
            case op_lstore_3: {
                store2Local<JLong>(3);
            } break;
            case op_fstore_0: {
                store2Local<JFloat>(0);
            } break;
            case op_fstore_1: {
                store2Local<JFloat>(1);
            } break;
            case op_fstore_2: {
                store2Local<JFloat>(2);
            } break;
            case op_fstore_3: {
                store2Local<JFloat>(3);
            } break;
            case op_dstore_0: {
                store2Local<JDouble>(0);
            } break;
            case op_dstore_1: {
                store2Local<JDouble>(1);
            } break;
            case op_dstore_2: {
                store2Local<JDouble>(2);
            } break;
            case op_dstore_3: {
                store2Local<JDouble>(3);
            } break;
            case op_astore_0: {
                store2Local<JRef>(0);
            } break;
            case op_astore_1: {
                store2Local<JRef>(1);
            } break;
            case op_astore_2: {
                store2Local<JRef>(2);
            } break;
            case op_astore_3: {
                store2Local<JRef>(3);
            } break;
            case op_iastore: {
                storeArrayItem<JInt>();
            } break;
            case op_lastore: {
                storeArrayItem<JLong>();
            } break;
            case op_fastore: {
                storeArrayItem<JFloat>();
            } break;
            case op_dastore: {
                storeArrayItem<JDouble>();
            } break;
            case op_aastore: {
                storeArrayItem<JRef>();
            } break;
            case op_bastore: {
                JInt *value = currentStackPop<JInt>();
                value->val = static_cast<int8_t>(value->val);

                JInt *index = currentStackPop<JInt>();
                JArray *arrayref = currentStackPop<JArray>();
                if (arrayref == nullptr) {
                    throw std::runtime_error("null pointer");
                }
                if (index->val > arrayref->length || index->val < 0) {
                    throw std::runtime_error("array index out of bounds");
                }
                yrt.jheap->putElement(*arrayref, index->val, value);

                delete index;
            } break;
            case op_sastore:
            case op_castore: {
                JInt *value = currentStackPop<JInt>();
                value->val = static_cast<int16_t>(value->val);

                JInt *index = currentStackPop<JInt>();
                JArray *arrayref = currentStackPop<JArray>();
                if (arrayref == nullptr) {
                    throw std::runtime_error("null pointer");
                }
                if (index->val > arrayref->length || index->val < 0) {
                    throw std::runtime_error("array index out of bounds");
                }
                yrt.jheap->putElement(*arrayref, index->val, value);

                delete index;
            } break;
            case op_pop: {
                delete currentStackPop<JType>();
            } break;
            case op_pop2: {
                delete currentStackPop<JType>();
                delete currentStackPop<JType>();
            } break;
            case op_dup: {
                JType *value = currentStackPop<JType>();

                assert(typeid(*value) != typeid(JLong) &&
                       typeid(*value) != typeid(JDouble));
                currentFrame->stack.push_back(value);
                currentFrame->stack.push_back(cloneValue(value));
            } break;
            case op_dup_x1: {
                JType *value1 = currentStackPop<JType>();
                JType *value2 = currentStackPop<JType>();

                assert(IS_COMPUTATIONAL_TYPE_1(value1));
                assert(IS_COMPUTATIONAL_TYPE_1(value2));

                currentFrame->stack.push_back(cloneValue(value1));
                currentFrame->stack.push_back(value2);
                currentFrame->stack.push_back(value1);
            } break;
            case op_dup_x2: {
                JType *value1 = currentStackPop<JType>();
                JType *value2 = currentStackPop<JType>();
                JType *value3 = currentStackPop<JType>();

                if (IS_COMPUTATIONAL_TYPE_1(value1) &&
                    IS_COMPUTATIONAL_TYPE_1(value2) &&
                    IS_COMPUTATIONAL_TYPE_1(value3)) {
                    // use structure 1
                    currentFrame->stack.push_back(cloneValue(value1));
                    currentFrame->stack.push_back(value3);
                    currentFrame->stack.push_back(value2);
                    currentFrame->stack.push_back(value1);
                } else if (IS_COMPUTATIONAL_TYPE_1(value1) &&
                           IS_COMPUTATIONAL_TYPE_2(value2)) {
                    // use structure 2
                    currentFrame->stack.push_back(value3);

                    currentFrame->stack.push_back(cloneValue(value1));
                    currentFrame->stack.push_back(value2);
                    currentFrame->stack.push_back(value1);
                } else {
                    SHOULD_NOT_REACH_HERE
                }
            } break;
            case op_dup2: {
                JType *value1 = currentStackPop<JType>();
                JType *value2 = currentStackPop<JType>();

                if (IS_COMPUTATIONAL_TYPE_1(value1) &&
                    IS_COMPUTATIONAL_TYPE_1(value2)) {
                    // use structure 1
                    currentFrame->stack.push_back(cloneValue(value2));
                    currentFrame->stack.push_back(cloneValue(value1));
                    currentFrame->stack.push_back(value2);
                    currentFrame->stack.push_back(value1);
                } else if (IS_COMPUTATIONAL_TYPE_2(value1)) {
                    // use structure 2
                    currentFrame->stack.push_back(value2);

                    currentFrame->stack.push_back(cloneValue(value1));
                    currentFrame->stack.push_back(value1);
                } else {
                    SHOULD_NOT_REACH_HERE
                }
            } break;
            case op_dup2_x1: {
                JType *value1 = currentStackPop<JType>();
                JType *value2 = currentStackPop<JType>();
                JType *value3 = currentStackPop<JType>();

                if (IS_COMPUTATIONAL_TYPE_1(value1) &&
                    IS_COMPUTATIONAL_TYPE_1(value2) &&
                    IS_COMPUTATIONAL_TYPE_1(value3)) {
                    // use structure 1
                    currentFrame->stack.push_back(cloneValue(value2));
                    currentFrame->stack.push_back(cloneValue(value1));
                    currentFrame->stack.push_back(value3);
                    currentFrame->stack.push_back(value2);
                    currentFrame->stack.push_back(value1);
                } else if (IS_COMPUTATIONAL_TYPE_2(value1) &&
                           IS_COMPUTATIONAL_TYPE_1(value2)) {
                    // use structure 2
                    currentFrame->stack.push_back(value3);

                    currentFrame->stack.push_back(cloneValue(value1));
                    currentFrame->stack.push_back(value2);
                    currentFrame->stack.push_back(value1);
                } else {
                    SHOULD_NOT_REACH_HERE
                }
            } break;
            case op_dup2_x2: {
                JType *value1 = currentStackPop<JType>();
                JType *value2 = currentStackPop<JType>();
                JType *value3 = currentStackPop<JType>();
                JType *value4 = currentStackPop<JType>();
                if (IS_COMPUTATIONAL_TYPE_1(value1) &&
                    IS_COMPUTATIONAL_TYPE_1(value2) &&
                    IS_COMPUTATIONAL_TYPE_1(value3) &&
                    IS_COMPUTATIONAL_TYPE_1(value4)) {
                    // use structure 1
                    currentFrame->stack.push_back(cloneValue(value2));
                    currentFrame->stack.push_back(cloneValue(value1));
                    currentFrame->stack.push_back(value4);
                    currentFrame->stack.push_back(value3);
                    currentFrame->stack.push_back(value2);
                    currentFrame->stack.push_back(value1);
                } else if (IS_COMPUTATIONAL_TYPE_2(value1) &&
                           IS_COMPUTATIONAL_TYPE_1(value2) &&
                           IS_COMPUTATIONAL_TYPE_1(value3)) {
                    // use structure 2
                    currentFrame->stack.push_back(value4);

                    currentFrame->stack.push_back(cloneValue(value1));
                    currentFrame->stack.push_back(value4);
                    currentFrame->stack.push_back(value2);
                    currentFrame->stack.push_back(value1);
                } else {
                    SHOULD_NOT_REACH_HERE
                }
            } break;
            case op_swap: {
                JType *value1 = currentStackPop<JType>();
                JType *value2 = currentStackPop<JType>();

                assert(IS_COMPUTATIONAL_TYPE_1(value1));
                assert(IS_COMPUTATIONAL_TYPE_1(value2));
                if (typeid(*value1) == typeid(JInt) &&
                    typeid(*value2) == typeid(JInt)) {
                    std::swap(value1, value2);
                } else if (typeid(*value1) == typeid(JInt) &&
                           typeid(*value2) == typeid(JFloat)) {
                    const int32_t temp = dynamic_cast<JInt *>(value1)->val;
                    dynamic_cast<JInt *>(value1)->val = static_cast<int32_t>(
                        dynamic_cast<JFloat *>(value2)->val);
                    dynamic_cast<JFloat *>(value2)->val =
                        static_cast<float>(temp);
                } else if (typeid(*value1) == typeid(JFloat) &&
                           typeid(*value2) == typeid(JInt)) {
                    const float temp = dynamic_cast<JFloat *>(value1)->val;
                    dynamic_cast<JFloat *>(value1)->val =
                        static_cast<int32_t>(dynamic_cast<JInt *>(value2)->val);
                    dynamic_cast<JInt *>(value2)->val =
                        static_cast<int32_t>(temp);
                } else if (typeid(*value1) == typeid(JFloat) &&
                           typeid(*value2) == typeid(JFloat)) {
                    std::swap(value1, value2);
                } else {
                    SHOULD_NOT_REACH_HERE
                }
            } break;
            case op_iadd: {
                binaryArithmetic<JInt>(std::plus<>());
            } break;
            case op_ladd: {
                binaryArithmetic<JLong>(std::plus<>());
            } break;
            case op_fadd: {
                binaryArithmetic<JFloat>(std::plus<>());
            } break;
            case op_dadd: {
                binaryArithmetic<JDouble>(std::plus<>());
            } break;
            case op_isub: {
                binaryArithmetic<JInt>(std::minus<>());
            } break;
            case op_lsub: {
                binaryArithmetic<JLong>(std::minus<>());
            } break;
            case op_fsub: {
                binaryArithmetic<JFloat>(std::minus<>());
            } break;
            case op_dsub: {
                binaryArithmetic<JDouble>(std::minus<>());
            } break;
            case op_imul: {
                binaryArithmetic<JInt>(std::multiplies<>());
            } break;
            case op_lmul: {
                binaryArithmetic<JLong>(std::multiplies<>());
            } break;
            case op_fmul: {
                binaryArithmetic<JFloat>(std::multiplies<>());
            } break;
            case op_dmul: {
                binaryArithmetic<JDouble>(std::multiplies<>());
            } break;
            case op_idiv: {
                binaryArithmetic<JInt>(std::divides<>());
            } break;
            case op_ldiv: {
                binaryArithmetic<JLong>(std::divides<>());
            } break;
            case op_fdiv: {
                binaryArithmetic<JFloat>(std::divides<>());
            } break;
            case op_ddiv: {
                binaryArithmetic<JDouble>(std::divides<>());

            } break;
            case op_irem: {
                binaryArithmetic<JInt>(std::modulus<>());
            } break;
            case op_lrem: {
                binaryArithmetic<JLong>(std::modulus<>());
            } break;
            case op_frem: {
                binaryArithmetic<JFloat>(std::fmod<float, float>);
            } break;
            case op_drem: {
                binaryArithmetic<JFloat>(std::fmod<double, double>);
            } break;
            case op_ineg: {
                unaryArithmetic<JInt>(std::negate<>());
            } break;
            case op_lneg: {
                unaryArithmetic<JLong>(std::negate<>());
            } break;
            case op_fneg: {
                unaryArithmetic<JFloat>(std::negate<>());
            } break;
            case op_dneg: {
                unaryArithmetic<JDouble>(std::negate<>());
            } break;
            case op_ishl: {
                binaryArithmetic<JInt>([](int32_t a, int32_t b) -> int32_t {
                    return a * std::pow(2, b & 0x1f);
                });
            } break;
            case op_lshl: {
                binaryArithmetic<JLong>([](int64_t a, int64_t b) -> int64_t {
                    return a * std::pow(2, b & 0x3f);
                });
            } break;
            case op_ishr: {
                binaryArithmetic<JInt>([](int32_t a, int32_t b) -> int32_t {
                    return std::floor(a / std::pow(2, b & 0x1f));
                });
            } break;
            case op_lshr: {
                binaryArithmetic<JLong>([](int64_t a, int64_t b) -> int64_t {
                    return std::floor(a / std::pow(2, b & 0x3f));
                });
            } break;
            case op_iushr: {
                binaryArithmetic<JInt>([](int32_t a, int32_t b) -> int32_t {
                    if (a > 0) {
                        return a >> (b & 0x1f);
                    } else if (a < 0) {
                        return (a >> (b & 0x1f)) + (2 << ~(b & 0x1f));
                    } else {
                        throw std::runtime_error("0 is not handled");
                    }
                });
            } break;
            case op_lushr: {
                binaryArithmetic<JLong>([](int64_t a, int64_t b) -> int64_t {
                    if (a > 0) {
                        return a >> (b & 0x3f);
                    } else if (a < 0) {
                        return (a >> (b & 0x1f)) + (2L << ~(b & 0x3f));
                    } else {
                        throw std::runtime_error("0 is not handled");
                    }
                });
            } break;
            case op_iand: {
                binaryArithmetic<JInt>(std::bit_and<>());
            } break;
            case op_land: {
                binaryArithmetic<JLong>(std::bit_and<>());
            } break;
            case op_ior: {
                binaryArithmetic<JInt>(std::bit_or<>());
            } break;
            case op_lor: {
                binaryArithmetic<JLong>(std::bit_or<>());
            } break;
            case op_ixor: {
                binaryArithmetic<JInt>(std::bit_xor<>());
            } break;
            case op_lxor: {
                binaryArithmetic<JLong>(std::bit_xor<>());
            } break;
            case op_iinc: {
                const u1 index = ext.code[++op];
                const int8_t count = ext.code[++op];
                const int32_t extendedCount = count;
                if (IS_JINT(currentFrame->locals[index])) {
                    dynamic_cast<JInt *>(currentFrame->locals[index])->val +=
                        extendedCount;
                } else if (IS_JLong(currentFrame->locals[index])) {
                    dynamic_cast<JLong *>(currentFrame->locals[index])->val +=
                        extendedCount;
                } else if (IS_JFloat(currentFrame->locals[index])) {
                    dynamic_cast<JFloat *>(currentFrame->locals[index])->val +=
                        extendedCount;
                } else if (IS_JDouble(currentFrame->locals[index])) {
                    dynamic_cast<JDouble *>(currentFrame->locals[index])->val +=
                        extendedCount;
                } else {
                    SHOULD_NOT_REACH_HERE
                }
            } break;
            case op_i2l: {
                typeCast<JInt, JLong>();
            } break;
            case op_i2f: {
                typeCast<JInt, JFloat>();
            } break;
            case op_i2d: {
                typeCast<JInt, JDouble>();
            } break;
            case op_l2i: {
                typeCast<JLong, JInt>();
            } break;
            case op_l2f: {
                typeCast<JLong, JFloat>();
            } break;
            case op_l2d: {
                typeCast<JLong, JDouble>();
            } break;
            case op_f2i: {
                typeCast<JFloat, JInt>();
            } break;
            case op_f2l: {
                typeCast<JFloat, JLong>();
            } break;
            case op_f2d: {
                typeCast<JFloat, JDouble>();
            } break;
            case op_d2i: {
                typeCast<JDouble, JInt>();
            } break;
            case op_d2l: {
                typeCast<JDouble, JLong>();
            } break;
            case op_d2f: {
                typeCast<JDouble, JFloat>();
            } break;
            case op_i2c:
            case op_i2b: {
                auto *value = currentStackPop<JInt>();
                auto *result = new JInt;
                result->val = (int8_t)(value->val);
                currentFrame->stack.push_back(result);
                delete value;
            } break;
            case op_i2s: {
                auto *value = currentStackPop<JInt>();
                auto *result = new JInt;
                result->val = (int16_t)(value->val);
                currentFrame->stack.push_back(result);
                delete value;
            } break;
            case op_lcmp: {
                auto *value2 = currentStackPop<JLong>();
                auto *value1 = currentStackPop<JLong>();
                if (value1->val > value2->val) {
                    auto *result = new JInt(1);
                    currentFrame->stack.push_back(result);
                } else if (value1->val == value2->val) {
                    auto *result = new JInt(0);
                    currentFrame->stack.push_back(result);
                } else {
                    auto *result = new JInt(-1);
                    currentFrame->stack.push_back(result);
                }
                delete value1;
                delete value2;
            } break;
            case op_fcmpg:
            case op_fcmpl: {
                auto *value2 = currentStackPop<JFloat>();
                auto *value1 = currentStackPop<JFloat>();
                if (value1->val > value2->val) {
                    auto *result = new JInt(1);
                    currentFrame->stack.push_back(result);
                } else if (std::abs(value1->val - value2->val) < 0.000001) {
                    auto *result = new JInt(0);
                    currentFrame->stack.push_back(result);
                } else {
                    auto *result = new JInt(-1);
                    currentFrame->stack.push_back(result);
                }
                delete value1;
                delete value2;
            } break;
            case op_dcmpl:
            case op_dcmpg: {
                auto *value2 = currentStackPop<JDouble>();
                auto *value1 = currentStackPop<JDouble>();
                if (value1->val > value2->val) {
                    auto *result = new JInt(1);
                    currentFrame->stack.push_back(result);
                } else if (std::abs(value1->val - value2->val) <
                           0.000000000001) {
                    auto *result = new JInt(0);
                    currentFrame->stack.push_back(result);
                } else {
                    auto *result = new JInt(-1);
                    currentFrame->stack.push_back(result);
                }
                delete value1;
                delete value2;
            } break;
            case op_ifeq: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value = currentStackPop<JInt>();
                if (value->val == 0) {
                    op = currentOffset + branchindex;
                }
                delete value;
            } break;
            case op_ifne: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value = currentStackPop<JInt>();
                if (value->val != 0) {
                    op = currentOffset + branchindex;
                }
                delete value;
            } break;
            case op_iflt: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value = currentStackPop<JInt>();
                if (value->val < 0) {
                    op = currentOffset + branchindex;
                }
                delete value;
            } break;
            case op_ifge: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value = currentStackPop<JInt>();
                if (value->val >= 0) {
                    op = currentOffset + branchindex;
                }
                delete value;
            } break;
            case op_ifgt: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value = currentStackPop<JInt>();
                if (value->val > 0) {
                    op = currentOffset + branchindex;
                }
                delete value;
            } break;
            case op_ifle: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value = currentStackPop<JInt>();
                if (value->val <= 0) {
                    op = currentOffset + branchindex;
                }
                delete value;
            } break;
            case op_if_icmpeq: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value2 = currentStackPop<JInt>();
                auto *value1 = currentStackPop<JInt>();
                if (value1->val == value2->val) {
                    op = currentOffset + branchindex;
                }
                delete value1;
                delete value2;
            } break;
            case op_if_icmpne: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value2 = currentStackPop<JInt>();
                auto *value1 = currentStackPop<JInt>();
                if (value1->val != value2->val) {
                    op = currentOffset + branchindex;
                }
                delete value1;
                delete value2;
            } break;
            case op_if_icmplt: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value2 = currentStackPop<JInt>();
                auto *value1 = currentStackPop<JInt>();
                if (value1->val < value2->val) {
                    op = currentOffset + branchindex;
                }
                delete value1;
                delete value2;
            } break;
            case op_if_icmpge: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value2 = currentStackPop<JInt>();
                auto *value1 = currentStackPop<JInt>();
                if (value1->val >= value2->val) {
                    op = currentOffset + branchindex;
                }
                delete value1;
                delete value2;
            } break;
            case op_if_icmpgt: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value2 = currentStackPop<JInt>();
                auto *value1 = currentStackPop<JInt>();
                if (value1->val > value2->val) {
                    op = currentOffset + branchindex;
                }
                delete value1;
                delete value2;
            } break;
            case op_if_icmple: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value2 = currentStackPop<JInt>();
                auto *value1 = currentStackPop<JInt>();
                if (value1->val <= value2->val) {
                    op = currentOffset + branchindex;
                }
                delete value1;
                delete value2;
            } break;
            case op_if_acmpeq: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value2 = currentStackPop<JObject>();
                auto *value1 = currentStackPop<JObject>();
                if (value1->offset == value2->offset &&
                    value1->jc == value2->jc) {
                    op = currentOffset + branchindex;
                }
                delete value1;
                delete value2;
            } break;
            case op_if_acmpne: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                auto *value2 = currentStackPop<JObject>();
                auto *value1 = currentStackPop<JObject>();
                if (value1->offset != value2->offset ||
                    value1->jc != value2->jc) {
                    op = currentOffset + branchindex;
                }
                delete value1;
                delete value2;
            } break;
            case op_goto: {
                u4 currentOffset = op - 1;
                int16_t branchindex = consumeU2(ext.code, op);
                op = currentOffset + branchindex;
            } break;
            case op_jsr: {
                throw std::runtime_error("unsupported opcode [jsr]");
            } break;
            case op_ret: {
                throw std::runtime_error("unsupported opcode [ret]");
            } break;
            case op_tableswitch: {
                u4 currentOffset = op - 1;
                op++;
                op++;
                op++;  // 3 bytes padding
                int32_t defaultIndex = consumeU4(ext.code, op);
                int32_t low = consumeU4(ext.code, op);
                int32_t high = consumeU4(ext.code, op);
                std::vector<int32_t> jumpOffset;
                FOR_EACH(i, high - low + 1) {
                    jumpOffset.push_back(consumeU4(ext.code, op));
                }

                auto *index = currentStackPop<JInt>();
                if (index->val < low || index->val > high) {
                    op = currentOffset + defaultIndex;
                } else {
                    op = currentOffset + jumpOffset[index->val - low];
                }
                delete index;
            } break;
            case op_lookupswitch: {
                u4 currentOffset = op - 1;
                op++;
                op++;
                op++;  // 3 bytes padding
                int32_t defaultIndex = consumeU4(ext.code, op);
                int32_t npair = consumeU4(ext.code, op);
                std::map<int32_t, int32_t> matchOffset;
                FOR_EACH(i, npair) {
                    matchOffset.insert(std::make_pair(consumeU4(ext.code, op),
                                                      consumeU4(ext.code, op)));
                }
                auto *key = currentStackPop<JInt>();
                auto res = matchOffset.find(key->val);
                if (res != matchOffset.end()) {
                    op = currentOffset + (*res).second;
                } else {
                    op = currentOffset + defaultIndex;
                }
                delete key;
            } break;
            case op_ireturn: {
                return flowReturn<JInt>();
            } break;
            case op_lreturn: {
                return flowReturn<JLong>();
            } break;
            case op_freturn: {
                return flowReturn<JFloat>();
            } break;
            case op_dreturn: {
                return flowReturn<JDouble>();
            } break;
            case op_areturn: {
                return flowReturn<JType>();
            } break;
            case op_return: {
                return nullptr;
            } break;
            case op_getstatic: {
                const u2 index = consumeU2(ext.code, op);
                auto symbolicRef = parseFieldSymbolicReference(jc, index);
                JType *field = cloneValue(getStaticField(
                    std::get<0>(symbolicRef), std::get<1>(symbolicRef),
                    std::get<2>(symbolicRef)));
                currentFrame->stack.push_back(field);
            } break;
            case op_putstatic: {
                u2 index = consumeU2(ext.code, op);
                JType *value = currentStackPop<JType>();
                auto symbolicRef = parseFieldSymbolicReference(jc, index);
                putStaticField(std::get<0>(symbolicRef),
                               std::get<1>(symbolicRef),
                               std::get<2>(symbolicRef), value);
            } break;
            case op_getfield: {
                u2 index = consumeU2(ext.code, op);
                JObject *objectref = currentStackPop<JObject>();
                auto symbolicRef = parseFieldSymbolicReference(jc, index);
                JType *field = cloneValue(yrt.jheap->getFieldByName(
                    std::get<0>(symbolicRef), std::get<1>(symbolicRef),
                    std::get<2>(symbolicRef), objectref));
                currentFrame->stack.push_back(field);

                delete objectref;
            } break;
            case op_putfield: {
                const u2 index = consumeU2(ext.code, op);
                JType *value = currentStackPop<JType>();
                JObject *objectref = currentStackPop<JObject>();
                auto symbolicRef = parseFieldSymbolicReference(jc, index);
                yrt.jheap->putFieldByName(
                    std::get<0>(symbolicRef), std::get<1>(symbolicRef),
                    std::get<2>(symbolicRef), objectref, value);

                delete objectref;
            } break;
            case op_invokevirtual: {
                const u2 index = consumeU2(ext.code, op);
                assert(typeid(*jc->raw.constPoolInfo[index]) ==
                       typeid(CONSTANT_Methodref));

                auto symbolicRef = parseMethodSymbolicReference(jc, index);

                if (std::get<1>(symbolicRef) == "<init>") {
                    std::runtime_error(
                        "invoking method should not be instance "
                        "initialization method\n");
                }
                if (!IS_SIGNATURE_POLYMORPHIC_METHOD(
                        std::get<0>(symbolicRef)->getClassName(),
                        std::get<1>(symbolicRef))) {
                    invokeVirtual(std::get<1>(symbolicRef),
                                  std::get<2>(symbolicRef));
                } else {
                    // TODO:TO BE IMPLEMENTED
                }

            } break;
            case op_invokespecial: {
                const u2 index = consumeU2(ext.code, op);
                std::tuple<JavaClass *, std::string, std::string> symbolicRef;

                if (typeid(*jc->raw.constPoolInfo[index]) ==
                    typeid(CONSTANT_InterfaceMethodref)) {
                    symbolicRef =
                        parseInterfaceMethodSymbolicReference(jc, index);
                } else if (typeid(*jc->raw.constPoolInfo[index]) ==
                           typeid(CONSTANT_Methodref)) {
                    symbolicRef = parseMethodSymbolicReference(jc, index);
                } else {
                    SHOULD_NOT_REACH_HERE
                }

                // If all of the following are true, let C be the direct
                // superclass of the current class :
                JavaClass *symbolicRefClass = std::get<0>(symbolicRef);
                if ("<init>" != std::get<1>(symbolicRef)) {
                    if (!IS_CLASS_INTERFACE(
                            symbolicRefClass->raw.accessFlags)) {
                        if (symbolicRefClass->getClassName() ==
                            jc->getSuperClassName()) {
                            if (IS_CLASS_SUPER(jc->raw.accessFlags)) {
                                invokeSpecial(yrt.ma->findJavaClass(
                                                  jc->getSuperClassName()),
                                              std::get<1>(symbolicRef),
                                              std::get<2>(symbolicRef));
                                break;
                            }
                        }
                    }
                }
                // Otherwise let C be the symbolic reference class
                invokeSpecial(std::get<0>(symbolicRef),
                              std::get<1>(symbolicRef),
                              std::get<2>(symbolicRef));
            } break;
            case op_invokestatic: {
                // Invoke a class (static) method
                const u2 index = consumeU2(ext.code, op);

                if (typeid(*jc->raw.constPoolInfo[index]) ==
                    typeid(CONSTANT_InterfaceMethodref)) {
                    auto symbolicRef =
                        parseInterfaceMethodSymbolicReference(jc, index);
                    invokeStatic(std::get<0>(symbolicRef),
                                 std::get<1>(symbolicRef),
                                 std::get<2>(symbolicRef));
                } else if (typeid(*jc->raw.constPoolInfo[index]) ==
                           typeid(CONSTANT_Methodref)) {
                    auto symbolicRef = parseMethodSymbolicReference(jc, index);
                    invokeStatic(std::get<0>(symbolicRef),
                                 std::get<1>(symbolicRef),
                                 std::get<2>(symbolicRef));
                } else {
                    SHOULD_NOT_REACH_HERE
                }
            } break;
            case op_invokeinterface: {
                const u2 index = consumeU2(ext.code, op);
                ++op;  // read count and discard
                ++op;  // opcode padding 0;

                if (typeid(*jc->raw.constPoolInfo[index]) ==
                    typeid(CONSTANT_InterfaceMethodref)) {
                    auto symbolicRef =
                        parseInterfaceMethodSymbolicReference(jc, index);
                    invokeInterface(std::get<0>(symbolicRef),
                                    std::get<1>(symbolicRef),
                                    std::get<2>(symbolicRef));
                }
            } break;
            case op_invokedynamic: {
                throw std::runtime_error("unsupported opcode [invokedynamic]");
            } break;
            case op_new: {
                const u2 index = consumeU2(ext.code, op);
                JObject *objectref = execNew(jc, index);
                currentFrame->stack.push_back(objectref);
            } break;
            case op_newarray: {
                const u1 atype = ext.code[++op];
                JInt *count = currentStackPop<JInt>();

                if (count->val < 0) {
                    throw std::runtime_error("negative array size");
                }
                JArray *arrayref = yrt.jheap->createPODArray(atype, count->val);

                currentFrame->stack.push_back(arrayref);
                delete count;
            } break;
            case op_anewarray: {
                const u2 index = consumeU2(ext.code, op);
                auto symbolicRef = parseClassSymbolicReference(jc, index);
                JInt *count = currentStackPop<JInt>();

                if (count->val < 0) {
                    throw std::runtime_error("negative array size");
                }
                JArray *arrayref = yrt.jheap->createObjectArray(
                    *std::get<0>(symbolicRef), count->val);

                currentFrame->stack.push_back(arrayref);
                delete count;
            } break;
            case op_arraylength: {
                JArray *arrayref = currentStackPop<JArray>();

                if (arrayref == nullptr) {
                    throw std::runtime_error("null pointer\n");
                }
                JInt *length = new JInt;
                length->val = arrayref->length;
                currentFrame->stack.push_back(length);

                delete arrayref;
            } break;
            case op_athrow: {
                auto *throwableObject = currentStackPop<JObject>();
                if (throwableObject == nullptr) {
                    throw std::runtime_error("null pointer");
                }
                if (!hasInheritanceRelationship(
                        throwableObject->jc,
                        yrt.ma->loadClassIfAbsent("java/lang/Throwable"))) {
                    throw std::runtime_error("it's not a throwable object");
                }

                if (handleException(jc, ext, throwableObject, op)) {
                    while (!currentFrame->stack.empty()) {
                        auto *temp = currentStackPop<JType>();
                        delete temp;
                    }
                    currentFrame->stack.push_back(throwableObject);
                } else /* Exception can not handled within method handlers */ {
                    exception.markException();
                    exception.setThrowExceptionInfo(throwableObject);
                    return throwableObject;
                }
            } break;
            case op_checkcast: {
                throw std::runtime_error("unsupported opcode [checkcast]");
            } break;
            case op_instanceof: {
                const u2 index = consumeU2(ext.code, op);
                auto *objectref = currentStackPop<JObject>();
                if (objectref == nullptr) {
                    currentFrame->stack.push_back(new JInt(0));
                }
                if (checkInstanceof(jc, index, objectref)) {
                    currentFrame->stack.push_back(new JInt(1));
                } else {
                    currentFrame->stack.push_back(new JInt(0));
                }
            } break;
            case op_monitorenter: {
                JType *ref = currentStackPop<JType>();

                if (ref == nullptr) {
                    throw std::runtime_error("null pointer");
                }

                if (!yrt.jheap->hasMonitor(ref)) {
                    dynamic_cast<JObject *>(ref)->offset =
                        yrt.jheap->createMonitor();
                }
                yrt.jheap->findMonitor(ref)->enter(std::this_thread::get_id());
            } break;
            case op_monitorexit: {
                JType *ref = currentStackPop<JType>();

                if (ref == nullptr) {
                    throw std::runtime_error("null pointer");
                }
                if (!yrt.jheap->hasMonitor(ref)) {
                    dynamic_cast<JObject *>(ref)->offset =
                        yrt.jheap->createMonitor();
                }
                yrt.jheap->findMonitor(ref)->exit();

            } break;
            case op_wide: {
                throw std::runtime_error("unsupported opcode [wide]");
            } break;
            case op_multianewarray: {
                throw std::runtime_error("unsupported opcode [multianewarray]");
            } break;
            case op_ifnull: {
                u4 currentOffset = op - 1;
                int16_t branchIndex = consumeU2(ext.code, op);
                JObject *value = currentStackPop<JObject>();
                if (value == nullptr) {
                    delete value;
                    op = currentOffset + branchIndex;
                }
            } break;
            case op_ifnonnull: {
                u4 currentOffset = op - 1;
                int16_t branchIndex = consumeU2(ext.code, op);
                JObject *value = currentStackPop<JObject>();
                if (value != nullptr) {
                    delete value;
                    op = currentOffset + branchIndex;
                }
            } break;
            case op_goto_w: {
                u4 currentOffset = op - 1;
                int32_t branchIndex = consumeU4(ext.code, op);
                op = currentOffset + branchIndex;
            } break;
            case op_jsr_w: {
                throw std::runtime_error("unsupported opcode [jsr_w]");
            } break;
            case op_breakpoint:
            case op_impdep1:
            case op_impdep2: {
                // Reserved opcodde
                std::cerr
                    << "Are you a dot.class hacker? Or you were entered a "
                       "strange region.";
                std::exit(EXIT_FAILURE);
            } break;
            default:
                std::cerr
                    << "The YVM can not recognize this opcode. Bytecode file "
                       "was be corrupted.";
                std::exit(EXIT_FAILURE);
        }
    }
    return nullptr;
}

//  This function does "ldc" opcode jc type of JavaClass, which indicate where
//  to resolve
void Interpreter::loadConstantPoolItem2Stack(const JavaClass *jc, u2 index) {
    if (typeid(*jc->raw.constPoolInfo[index]) == typeid(CONSTANT_Integer)) {
        auto val =
            dynamic_cast<CONSTANT_Integer *>(jc->raw.constPoolInfo[index])->val;
        JInt *ival = new JInt;
        ival->val = val;
        currentFrame->stack.push_back(ival);
    } else if (typeid(*jc->raw.constPoolInfo[index]) ==
               typeid(CONSTANT_Float)) {
        auto val =
            dynamic_cast<CONSTANT_Float *>(jc->raw.constPoolInfo[index])->val;
        JFloat *fval = new JFloat;
        fval->val = val;
        currentFrame->stack.push_back(fval);
    } else if (typeid(*jc->raw.constPoolInfo[index]) ==
               typeid(CONSTANT_String)) {
        auto val = jc->getString(
            dynamic_cast<CONSTANT_String *>(jc->raw.constPoolInfo[index])
                ->stringIndex);
        JObject *str = yrt.jheap->createObject(
            *yrt.ma->loadClassIfAbsent("java/lang/String"));
        JArray *value = yrt.jheap->createCharArray(val, val.length());
        // Put string  into str's field; according the source file of
        // java.lang.Object, we know that its first field was used to store
        // chars
        yrt.jheap->putFieldByOffset(*str, 0, value);
        currentFrame->stack.push_back(str);
    } else if (typeid(*jc->raw.constPoolInfo[index]) ==
               typeid(CONSTANT_Class)) {
        throw std::runtime_error("nonsupport region");
    } else if (typeid(*jc->raw.constPoolInfo[index]) ==
               typeid(CONSTANT_MethodType)) {
        throw std::runtime_error("nonsupport region");
    } else if (typeid(*jc->raw.constPoolInfo[index]) ==
               typeid(CONSTANT_MethodHandle)) {
        throw std::runtime_error("nonsupport region");
    } else {
        throw std::runtime_error(
            "invalid symbolic reference index on constant "
            "pool");
    }
}

bool Interpreter::handleException(const JavaClass *jc, const CodeAttrCore &ext,
                                  const JObject *objectref, u4 &op) {
    FOR_EACH(i, ext.exceptionTableLength) {
        const std::string &catchTypeName = jc->getString(
            dynamic_cast<CONSTANT_Class *>(
                jc->raw.constPoolInfo[ext.exceptionTable[i].catchType])
                ->nameIndex);

        if (hasInheritanceRelationship(
                yrt.ma->findJavaClass(objectref->jc->getClassName()),
                yrt.ma->findJavaClass(catchTypeName)) &&
            ext.exceptionTable[i].startPC <= op &&
            op < ext.exceptionTable[i].endPC) {
            // start<=op<end
            // If we found a proper exception handler, set current pc as
            // handlerPC of this exception table item;
            op = ext.exceptionTable[i].handlerPC - 1;
            return true;
        }
        if (ext.exceptionTable[i].catchType == 0) {
            op = ext.exceptionTable[i].handlerPC - 1;
            return true;
        }
    }

    return false;
}

std::tuple<JavaClass *, std::string, std::string>
Interpreter::parseFieldSymbolicReference(const JavaClass *jc, u2 index) const {
    const std::string &symbolicReferenceFieldName = jc->getString(
        dynamic_cast<CONSTANT_NameAndType *>(
            jc->raw.constPoolInfo[dynamic_cast<CONSTANT_Fieldref *>(
                                      jc->raw.constPoolInfo[index])
                                      ->nameAndTypeIndex])
            ->nameIndex);

    const std::string &symbolicReferenceFieldDescriptor = jc->getString(
        dynamic_cast<CONSTANT_NameAndType *>(
            jc->raw.constPoolInfo[dynamic_cast<CONSTANT_Fieldref *>(
                                      jc->raw.constPoolInfo[index])
                                      ->nameAndTypeIndex])
            ->descriptorIndex);

    JavaClass *symbolicReferenceClass = yrt.ma->loadClassIfAbsent(jc->getString(
        dynamic_cast<CONSTANT_Class *>(
            jc->raw.constPoolInfo[dynamic_cast<CONSTANT_Fieldref *>(
                                      jc->raw.constPoolInfo[index])
                                      ->classIndex])
            ->nameIndex));

    yrt.ma->linkClassIfAbsent(symbolicReferenceClass->getClassName());

    return std::make_tuple(symbolicReferenceClass, symbolicReferenceFieldName,
                           symbolicReferenceFieldDescriptor);
}

std::tuple<JavaClass *, std::string, std::string>
Interpreter::parseInterfaceMethodSymbolicReference(const JavaClass *jc,
                                                   u2 index) const {
    const std::string &symbolicReferenceInterfaceMethodName = jc->getString(
        dynamic_cast<CONSTANT_NameAndType *>(
            jc->raw.constPoolInfo[dynamic_cast<CONSTANT_InterfaceMethodref *>(
                                      jc->raw.constPoolInfo[index])
                                      ->nameAndTypeIndex])
            ->nameIndex);

    const std::string &symbolicReferenceInterfaceMethodDescriptor =
        jc->getString(
            dynamic_cast<CONSTANT_NameAndType *>(
                jc->raw
                    .constPoolInfo[dynamic_cast<CONSTANT_InterfaceMethodref *>(
                                       jc->raw.constPoolInfo[index])
                                       ->nameAndTypeIndex])
                ->descriptorIndex);

    JavaClass *symbolicReferenceInterfaceMethodClass =
        yrt.ma->loadClassIfAbsent(jc->getString(
            dynamic_cast<CONSTANT_Class *>(
                jc->raw
                    .constPoolInfo[dynamic_cast<CONSTANT_InterfaceMethodref *>(
                                       jc->raw.constPoolInfo[index])
                                       ->classIndex])
                ->nameIndex));
    yrt.ma->linkClassIfAbsent(
        symbolicReferenceInterfaceMethodClass->getClassName());

    return std::make_tuple(symbolicReferenceInterfaceMethodClass,
                           symbolicReferenceInterfaceMethodName,
                           symbolicReferenceInterfaceMethodDescriptor);
}

std::tuple<JavaClass *, std::string, std::string>
Interpreter::parseMethodSymbolicReference(const JavaClass *jc, u2 index) const {
    const std::string &symbolicReferenceMethodName = jc->getString(
        dynamic_cast<CONSTANT_NameAndType *>(
            jc->raw.constPoolInfo[dynamic_cast<CONSTANT_Methodref *>(
                                      jc->raw.constPoolInfo[index])
                                      ->nameAndTypeIndex])
            ->nameIndex);

    const std::string &symbolicReferenceMethodDescriptor = jc->getString(
        dynamic_cast<CONSTANT_NameAndType *>(
            jc->raw.constPoolInfo[dynamic_cast<CONSTANT_Methodref *>(
                                      jc->raw.constPoolInfo[index])
                                      ->nameAndTypeIndex])
            ->descriptorIndex);

    JavaClass *symbolicReferenceMethodClass =
        yrt.ma->loadClassIfAbsent(jc->getString(
            dynamic_cast<CONSTANT_Class *>(
                jc->raw.constPoolInfo[dynamic_cast<CONSTANT_Methodref *>(
                                          jc->raw.constPoolInfo[index])
                                          ->classIndex])
                ->nameIndex));
    yrt.ma->linkClassIfAbsent(symbolicReferenceMethodClass->getClassName());

    return std::make_tuple(symbolicReferenceMethodClass,
                           symbolicReferenceMethodName,
                           symbolicReferenceMethodDescriptor);
}

std::tuple<JavaClass *> Interpreter::parseClassSymbolicReference(
    const JavaClass *jc, u2 index) const {
    const std::string &ref = jc->getString(
        dynamic_cast<CONSTANT_Class *>(jc->raw.constPoolInfo[index])
            ->nameIndex);
    std::string str{ref};
    if (ref[0] == '[') {
        str = peelArrayComponentTypeFrom(ref);
    }
    return std::make_tuple(yrt.ma->loadClassIfAbsent(str));
}

JType *Interpreter::getStaticField(JavaClass *parsedJc,
                                   const std::string &fieldName,
                                   const std::string &fieldDescriptor) {
    yrt.ma->linkClassIfAbsent(parsedJc->getClassName());
    yrt.ma->initClassIfAbsent(*this, parsedJc->getClassName());

    FOR_EACH(i, parsedJc->raw.fieldsCount) {
        if (IS_FIELD_STATIC(parsedJc->raw.fields[i].accessFlags)) {
            const std::string &n =
                parsedJc->getString(parsedJc->raw.fields[i].nameIndex);
            const std::string &d =
                parsedJc->getString(parsedJc->raw.fields[i].descriptorIndex);
            if (n == fieldName && d == fieldDescriptor) {
                return parsedJc->sfield.find(i)->second;
            }
        }
    }
    if (parsedJc->raw.superClass != 0) {
        return getStaticField(
            yrt.ma->findJavaClass(parsedJc->getSuperClassName()), fieldName,
            fieldDescriptor);
    }
    return nullptr;
}

void Interpreter::putStaticField(JavaClass *parsedJc,
                                 const std::string &fieldName,
                                 const std::string &fieldDescriptor,
                                 JType *value) {
    yrt.ma->linkClassIfAbsent(parsedJc->getClassName());
    yrt.ma->initClassIfAbsent(*this, parsedJc->getClassName());

    FOR_EACH(i, parsedJc->raw.fieldsCount) {
        if (IS_FIELD_STATIC(parsedJc->raw.fields[i].accessFlags)) {
            const std::string &n =
                parsedJc->getString(parsedJc->raw.fields[i].nameIndex);
            const std::string &d =
                parsedJc->getString(parsedJc->raw.fields[i].descriptorIndex);
            if (n == fieldName && d == fieldDescriptor) {
                parsedJc->sfield.find(i)->second = value;
                return;
            }
        }
    }
    if (parsedJc->raw.superClass != 0) {
        putStaticField(yrt.ma->findJavaClass(parsedJc->getSuperClassName()),
                       fieldName, fieldDescriptor, value);
    }
}

JObject *Interpreter::execNew(const JavaClass *jc, u2 index) {
    yrt.ma->linkClassIfAbsent(const_cast<JavaClass *>(jc)->getClassName());
    yrt.ma->initClassIfAbsent(*this,
                              const_cast<JavaClass *>(jc)->getClassName());

    if (typeid(*jc->raw.constPoolInfo[index]) != typeid(CONSTANT_Class)) {
        throw std::runtime_error(
            "operand index of new is not a class or "
            "interface\n");
    }
    std::string className = jc->getString(
        dynamic_cast<CONSTANT_Class *>(jc->raw.constPoolInfo[index])
            ->nameIndex);
    JavaClass *newClass = yrt.ma->loadClassIfAbsent(className);
    return yrt.jheap->createObject(*newClass);
}

CodeAttrCore Interpreter::getCodeAttrCore(const MethodInfo *m) {
    CodeAttrCore ext{};
    if (!m) {
        return ext;
    }

    FOR_EACH(i, m->attributeCount) {
        if (typeid(*m->attributes[i]) == typeid(ATTR_Code)) {
            ext.code = dynamic_cast<ATTR_Code *>(m->attributes[i])->code;
            ext.codeLength = ((ATTR_Code *)m->attributes[i])->codeLength;
            ext.maxLocal =
                dynamic_cast<ATTR_Code *>(m->attributes[i])->maxLocals;
            ext.maxStack =
                dynamic_cast<ATTR_Code *>(m->attributes[i])->maxStack;
            ext.exceptionTableLength =
                dynamic_cast<ATTR_Code *>(m->attributes[i])
                    ->exceptionTableLength;
            ext.exceptionTable =
                dynamic_cast<ATTR_Code *>(m->attributes[i])->exceptionTable;
            ext.valid = true;
            break;
        }
    }
    return ext;
}

bool Interpreter::checkInstanceof(const JavaClass *jc, u2 index,
                                  JType *objectref) {
    std::string TclassName =
        (char *)dynamic_cast<CONSTANT_Utf8 *>(
            jc->raw.constPoolInfo[dynamic_cast<CONSTANT_Class *>(
                                      jc->raw.constPoolInfo[index])
                                      ->nameIndex])
            ->bytes;
    constexpr short TYPE_ARRAY = 1;
    constexpr short TYPE_CLASS = 2;
    constexpr short TYPE_INTERFACE = 3;

    short tType = 0;
    if (TclassName.find('[') != std::string::npos) {
        tType = TYPE_ARRAY;
    } else {
        if (IS_CLASS_INTERFACE(
                yrt.ma->findJavaClass(TclassName)->raw.accessFlags)) {
            tType = TYPE_INTERFACE;
        } else {
            tType = TYPE_CLASS;
        }
    }

    if (typeid(objectref) == typeid(JObject)) {
        if (!IS_CLASS_INTERFACE(
                dynamic_cast<JObject *>(objectref)->jc->raw.accessFlags)) {
            // If it's an ordinary class
            if (tType == TYPE_CLASS) {
                if (yrt.ma->findJavaClass(dynamic_cast<JObject *>(objectref)
                                              ->jc->getClassName())
                            ->getClassName() == TclassName ||
                    yrt.ma->findJavaClass(dynamic_cast<JObject *>(objectref)
                                              ->jc->getSuperClassName())
                            ->getClassName() == TclassName) {
                    return true;
                }
            } else if (tType == TYPE_INTERFACE) {
                auto &&interfaceIdxs = dynamic_cast<JObject *>(objectref)
                                           ->jc->getInterfacesIndex();
                FOR_EACH(i, interfaceIdxs.size()) {
                    std::string interfaceName =
                        dynamic_cast<JObject *>(objectref)->jc->getString(
                            dynamic_cast<CONSTANT_Class *>(
                                dynamic_cast<JObject *>(objectref)
                                    ->jc->raw.constPoolInfo[interfaceIdxs[i]])
                                ->nameIndex);
                    if (interfaceName == TclassName) {
                        return true;
                    }
                }
            } else {
                SHOULD_NOT_REACH_HERE
            }
        } else {
            // Otherwise, it's an interface class
            if (tType == TYPE_CLASS) {
                if (TclassName == "java/lang/Object") {
                    return true;
                }
            } else if (tType == TYPE_INTERFACE) {
                if (TclassName == dynamic_cast<JObject *>(objectref)
                                      ->jc->getClassName() ||
                    TclassName == dynamic_cast<JObject *>(objectref)
                                      ->jc->getSuperClassName()) {
                    return true;
                }
            } else {
                SHOULD_NOT_REACH_HERE
            }
        }
    } else if (typeid(objectref) == typeid(JArray)) {
        if (tType == TYPE_CLASS) {
            if (TclassName == "java/lang/Object") {
                return true;
            }
        } else if (tType == TYPE_INTERFACE) {
            auto *firstComponent = dynamic_cast<JObject *>(
                yrt.jheap->getElement(*dynamic_cast<JArray *>(objectref), 0));
            auto &&interfaceIdxs = firstComponent->jc->getInterfacesIndex();
            FOR_EACH(i, interfaceIdxs.size()) {
                if (firstComponent->jc->getString(
                        dynamic_cast<CONSTANT_Class *>(
                            firstComponent->jc->raw
                                .constPoolInfo[interfaceIdxs[i]])
                            ->nameIndex) == TclassName) {
                    return true;
                }
            }
        } else if (tType == TYPE_ARRAY) {
            throw std::runtime_error("to be continue\n");
        } else {
            SHOULD_NOT_REACH_HERE
        }
    } else {
        SHOULD_NOT_REACH_HERE
    }
}

std::pair<MethodInfo *, const JavaClass *> Interpreter::findMethod(
    const JavaClass *jc, const std::string &methodName,
    const std::string &methodDescriptor) {
    // Find corresponding method at current object's class;
    FOR_EACH(i, jc->raw.methodsCount) {
        auto *methodInfo = jc->getMethod(methodName, methodDescriptor);
        if (methodInfo) {
            return std::make_pair(methodInfo, jc);
        }
    }
    // Find corresponding method at object's super class unless it's an instance
    // of
    if (jc->raw.superClass != 0) {
        JavaClass *superClass =
            yrt.ma->loadClassIfAbsent(jc->getSuperClassName());
        auto methodPair = findMethod(jc, methodName, methodDescriptor);
        if (methodPair.first) {
            return methodPair;
        }
    }
    // Find corresponding method at object's all interfaces if at least one
    // interface class existing
    if (jc->raw.interfacesCount > 0) {
        FOR_EACH(eachInterface, jc->raw.interfacesCount) {
            const std::string &interfaceName = jc->getString(
                dynamic_cast<CONSTANT_Class *>(
                    jc->raw.constPoolInfo[jc->raw.interfaces[eachInterface]])
                    ->nameIndex);
            JavaClass *interfaceClass =
                yrt.ma->loadClassIfAbsent(interfaceName);
            auto *methodInfo =
                interfaceClass->getMethod(methodName, methodDescriptor);
            if (methodInfo && (!IS_METHOD_ABSTRACT(methodInfo->accessFlags) &&
                               !IS_METHOD_STATIC(methodInfo->accessFlags) &&
                               !IS_METHOD_PRIVATE(methodInfo->accessFlags))) {
                return std::make_pair(methodInfo, interfaceClass);
            }
            if (methodInfo && (!IS_METHOD_STATIC(methodInfo->accessFlags) &&
                               !IS_METHOD_PRIVATE(methodInfo->accessFlags))) {
                return std::make_pair(methodInfo, interfaceClass);
            }
        }
    }
    // Otherwise, failed to find corresponding method by given method name and
    // method descriptor
    return std::make_pair(nullptr, nullptr);
}

void Interpreter::pushMethodArguments(Frame *frame,
                                      std::vector<int> &parameter) {
    for (int64_t i = (int64_t)parameter.size() - 1; i >= 0; i--) {
        if (parameter[i] == T_INT || parameter[i] == T_BOOLEAN ||
            parameter[i] == T_CHAR || parameter[i] == T_BYTE ||
            parameter[i] == T_SHORT) {
            auto *v = currentStackPop<JInt>();
            frame->locals.push_front(v);
        } else if (parameter[i] == T_FLOAT) {
            auto *v = currentStackPop<JFloat>();
            frame->locals.push_front(v);
        } else if (parameter[i] == T_DOUBLE) {
            auto *v = currentStackPop<JDouble>();
            frame->locals.push_front(nullptr);
            frame->locals.push_front(v);
        } else if (parameter[i] == T_LONG) {
            auto *v = currentStackPop<JLong>();
            frame->locals.push_front(nullptr);
            frame->locals.push_front(v);
        } else if (parameter[i] == T_EXTRA_ARRAY) {
            auto *v = currentStackPop<JArray>();
            frame->locals.push_front(v);
        } else if (parameter[i] == T_EXTRA_OBJECT) {
            auto *v = currentStackPop<JObject>();
            frame->locals.push_front(v);
        } else {
            SHOULD_NOT_REACH_HERE;
        }
    }
}

JObject *Interpreter::pushMethodThisArgument(Frame *frame) {
    auto *objectref = currentStackPop<JObject>();
    frame->locals.push_front(objectref);
    return objectref;
}

void Interpreter::invokeByName(JavaClass *jc, const std::string &methodName,
                               const std::string &methodDescriptor) {
    const MethodInfo *m = jc->getMethod(methodName, methodDescriptor);
    CodeAttrCore ext = getCodeAttrCore(m);
    const int returnType =
        std::get<0>(peelMethodParameterAndType(methodDescriptor));

    if (!ext.valid) {
#ifdef YVM_DEBUG_SHOW_EXEC_FLOW
        std::cout << "Method " << jc->getClassName() << "::" << methodName
                  << "() not found!\n";
#endif
        return;
    }

#ifdef YVM_DEBUG_SHOW_EXEC_FLOW
    for (int i = 0; i < frames.size(); i++) {
        std::cout << "-";
    }
    std::cout << "Execute " << jc->getClassName() << "::" << methodName << "() "
              << methodDescriptor << "\n";
#endif

    // Actual method calling routine
    Frame *frame = new Frame;
    frame->locals.resize(ext.maxLocal);
    frames.push_back(frame);
    currentFrame = frames.back();

    if (frame->locals.size() < ext.maxLocal) {
        frame->locals.resize(ext.maxLocal);
    }

    JType *returnValue{};
    if (IS_METHOD_NATIVE(m->accessFlags)) {
        returnValue = cloneValue(
            execNative(jc->getClassName(), methodName, methodDescriptor));
    } else {
        returnValue = cloneValue(execCode(jc, std::move(ext)));
    }
    popFrame();

    // Since invokeByName() was merely used to call <clinit> and main method of
    // running program, therefore, if an exception reached here, we don't need
    // to push its value into frame  again (In fact there is no more frame), we
    // just print stack trace inforamtion to notice user and return directly
    if (returnType != T_EXTRA_VOID) {
        currentFrame->stack.push_back(returnValue);
    }
    if (exception.hasUnhandledException()) {
        exception.extendExceptionStackTrace(methodName);
        exception.printStackTrace();
    }

    GC_SAFE_POINT
    if (yrt.gc->shallGC()) {
        yrt.gc->stopTheWorld();
        yrt.gc->gc(GCPolicy::GC_MARK_AND_SWEEP);
    }
}

void Interpreter::invokeInterface(const JavaClass *jc,
                                  const std::string &methodName,
                                  const std::string &methodDescriptor) {
    // Invoke interface method

    const auto invokingMethod = findMethod(jc, methodName, methodDescriptor);
#ifdef YVM_DEBUG_SHOW_EXEC_FLOW
    for (int i = 0; i < frames.size(); i++) {
        std::cout << "-";
    }
    std::cout << "Execute "
              << const_cast<JavaClass *>(invokingMethod.second)->getClassName()
              << "::" << methodName << "() " << methodDescriptor << "\n";
#endif

    if (invokingMethod.first == nullptr) {
        throw std::runtime_error("no such method existed");
    }

    auto parameterAndReturnType = peelMethodParameterAndType(methodDescriptor);
    const int returnType = std::get<0>(parameterAndReturnType);
    auto parameter = std::get<1>(parameterAndReturnType);
    Frame *frame = new Frame;
    pushMethodArguments(frame, parameter);
    pushMethodThisArgument(frame);

    CodeAttrCore ext = getCodeAttrCore(invokingMethod.first);
    frame->locals.resize(ext.maxLocal);
    frames.push_back(frame);
    this->currentFrame = frame;

    if (frame->locals.size() < ext.maxLocal) {
        frame->locals.resize(ext.maxLocal);
    }

    JType *returnValue{};
    if (IS_METHOD_NATIVE(invokingMethod.first->accessFlags)) {
        returnValue = cloneValue(execNative(
            const_cast<JavaClass *>(invokingMethod.second)->getClassName(),
            methodName, methodDescriptor));
    } else {
        returnValue =
            cloneValue(execCode(invokingMethod.second, std::move(ext)));
    }

    popFrame();
    if (returnType != T_EXTRA_VOID || exception.hasUnhandledException()) {
        currentFrame->stack.push_back(returnValue);
        if (exception.hasUnhandledException()) {
            exception.extendExceptionStackTrace(methodName);
        }
    }

    GC_SAFE_POINT
    if (yrt.gc->shallGC()) {
        yrt.gc->stopTheWorld();
        yrt.gc->gc(GCPolicy::GC_MARK_AND_SWEEP);
    }
}

void Interpreter::invokeVirtual(const std::string &methodName,
                                const std::string &methodDescriptor) {
    auto parameterAndReturnType = peelMethodParameterAndType(methodDescriptor);
    const int returnType = std::get<0>(parameterAndReturnType);
    auto parameter = std::get<1>(parameterAndReturnType);

    Frame *frame = new Frame;
    pushMethodArguments(frame, parameter);
    auto *objectref = pushMethodThisArgument(frame);
    frames.push_back(frame);
    this->currentFrame = frame;

    auto invokingMethod =
        findMethod(objectref->jc, methodName, methodDescriptor);
    auto ext = getCodeAttrCore(invokingMethod.first);

    if (frame->locals.size() < ext.maxLocal) {
        frame->locals.resize(ext.maxLocal);
    }

#ifdef YVM_DEBUG_SHOW_EXEC_FLOW
    for (int i = 0; i < frames.size(); i++) {
        std::cout << "-";
    }
    std::cout << "Execute "
              << const_cast<JavaClass *>(invokingMethod.second)->getClassName()
              << "::" << methodName << "() " << methodDescriptor << "\n";
#endif

    JType *returnValue{};
    if (invokingMethod.first) {
        if (IS_METHOD_NATIVE(invokingMethod.first->accessFlags)) {
            returnValue = cloneValue(execNative(
                const_cast<JavaClass *>(invokingMethod.second)->getClassName(),
                invokingMethod.second->getString(
                    invokingMethod.first->nameIndex),
                invokingMethod.second->getString(
                    invokingMethod.first->descriptorIndex)));
        } else {
            returnValue =
                cloneValue(execCode(invokingMethod.second, std::move(ext)));
        }
    } else {
        throw std::runtime_error("can not find method to call");
    }

    popFrame();
    if (returnType != T_EXTRA_VOID || exception.hasUnhandledException()) {
        currentFrame->stack.push_back(returnValue);
        if (exception.hasUnhandledException()) {
            exception.extendExceptionStackTrace(methodName);
        }
    }

    GC_SAFE_POINT
    if (yrt.gc->shallGC()) {
        yrt.gc->stopTheWorld();
        yrt.gc->gc(GCPolicy::GC_MARK_AND_SWEEP);
    }
}

void Interpreter::invokeSpecial(const JavaClass *jc,
                                const std::string &methodName,
                                const std::string &methodDescriptor) {
    //  Invoke instance method; special handling for superclass, private,
    //  and instance initialization method invocations

    auto parameterAndReturnType = peelMethodParameterAndType(methodDescriptor);
    const int returnType = std::get<0>(parameterAndReturnType);
    auto parameter = std::get<1>(parameterAndReturnType);

    Frame *frame = new Frame;
    pushMethodArguments(frame, parameter);
    auto *objectref = pushMethodThisArgument(frame);
    frames.push_back(frame);
    this->currentFrame = frame;

    const auto invokingMethod = findMethod(jc, methodName, methodDescriptor);
#ifdef YVM_DEBUG_SHOW_EXEC_FLOW
    for (int i = 0; i < frames.size(); i++) {
        std::cout << "-";
    }
    std::cout << "Execute "
              << const_cast<JavaClass *>(invokingMethod.second)->getClassName()
              << "::" << methodName << "() " << methodDescriptor << "\n";
#endif

    JType *returnValue{};
    if (invokingMethod.first) {
        if (IS_METHOD_NATIVE(invokingMethod.first->accessFlags)) {
            returnValue = cloneValue(execNative(
                const_cast<JavaClass *>(invokingMethod.second)->getClassName(),
                invokingMethod.second->getString(
                    invokingMethod.first->nameIndex),
                invokingMethod.second->getString(
                    invokingMethod.first->descriptorIndex)));
        } else {
            auto ext = getCodeAttrCore(invokingMethod.first);
            if (frame->locals.size() < ext.maxLocal) {
                frame->locals.resize(ext.maxLocal);
            }
            returnValue =
                cloneValue(execCode(invokingMethod.second, std::move(ext)));
        }
    } else if (IS_CLASS_INTERFACE(jc->raw.accessFlags)) {
        JavaClass *javaLangObjectClass = yrt.ma->findJavaClass(
            "java/lang/"
            "Object");
        MethodInfo *javaLangObjectMethod =
            javaLangObjectClass->getMethod(methodName, methodDescriptor);
        if (javaLangObjectMethod &&
            IS_METHOD_PUBLIC(javaLangObjectMethod->accessFlags) &&
            !IS_METHOD_STATIC(javaLangObjectMethod->accessFlags)) {
            if (IS_METHOD_NATIVE(javaLangObjectMethod->accessFlags)) {
                returnValue = cloneValue(
                    execNative(javaLangObjectClass->getClassName(),
                               javaLangObjectClass->getString(
                                   javaLangObjectMethod->nameIndex),
                               javaLangObjectClass->getString(
                                   javaLangObjectMethod->descriptorIndex)));
            } else {
                auto ext = getCodeAttrCore(javaLangObjectMethod);
                frame->locals.resize(ext.maxLocal);
                returnValue =
                    cloneValue(execCode(javaLangObjectClass, std::move(ext)));
            }
        }
    }

    popFrame();
    if (returnType != T_EXTRA_VOID || exception.hasUnhandledException()) {
        currentFrame->stack.push_back(returnValue);
        if (exception.hasUnhandledException()) {
            exception.extendExceptionStackTrace(methodName);
        }
    }

    GC_SAFE_POINT
    if (yrt.gc->shallGC()) {
        yrt.gc->stopTheWorld();
        yrt.gc->gc(GCPolicy::GC_MARK_AND_SWEEP);
    }
}

void Interpreter::invokeStatic(const JavaClass *jc,
                               const std::string &methodName,
                               const std::string &methodDescriptor) {
    // Get instance method name and descriptor from CONSTANT_Methodref locating
    // by index and get interface method parameter and return value descriptor
    yrt.ma->linkClassIfAbsent(const_cast<JavaClass *>(jc)->getClassName());
    yrt.ma->initClassIfAbsent(*this,
                              const_cast<JavaClass *>(jc)->getClassName());

    const auto invokingMethod = findMethod(jc, methodName, methodDescriptor);
#ifdef YVM_DEBUG_SHOW_EXEC_FLOW
    for (int i = 0; i < frames.size(); i++) {
        std::cout << "-";
    }
    std::cout << "Execute "
              << const_cast<JavaClass *>(invokingMethod.second)->getClassName()
              << "::" << methodName << "() " << methodDescriptor << "\n";
#endif
    assert(IS_METHOD_STATIC(invokingMethod.first->accessFlags) == true);
    assert(IS_METHOD_ABSTRACT(invokingMethod.first->accessFlags) == false);
    assert("<init>" != methodName);

    Frame *frame = new Frame;
    auto ext = getCodeAttrCore(invokingMethod.first);

    if (frame->locals.size() < ext.maxLocal) {
        frame->locals.resize(ext.maxLocal);
    }

    auto parameterAndReturnType = peelMethodParameterAndType(methodDescriptor);
    const int returnType = std::get<0>(parameterAndReturnType);
    auto parameter = std::get<1>(parameterAndReturnType);
    pushMethodArguments(frame, parameter);

    frames.push_back(frame);
    this->currentFrame = frame;

    JType *returnValue{};
    if (IS_METHOD_NATIVE(invokingMethod.first->accessFlags)) {
        returnValue = cloneValue(execNative(
            const_cast<JavaClass *>(invokingMethod.second)->getClassName(),
            methodName, methodDescriptor));
    } else {
        returnValue =
            cloneValue(execCode(invokingMethod.second, std::move(ext)));
    }
    popFrame();

    if (returnType != T_EXTRA_VOID || exception.hasUnhandledException()) {
        currentFrame->stack.push_back(returnValue);
        if (exception.hasUnhandledException()) {
            exception.extendExceptionStackTrace(methodName);
        }
    }

    GC_SAFE_POINT
    if (yrt.gc->shallGC()) {
        yrt.gc->stopTheWorld();
        yrt.gc->gc(GCPolicy::GC_MARK_AND_SWEEP);
    }
}

JType *Interpreter::execNative(const std::string &className,
                               const std::string &methodName,
                               const std::string &methodDescriptor) {
    std::string nativeMethod(className);
    nativeMethod.append(".");
    nativeMethod.append(methodName);
    nativeMethod.append(".");
    nativeMethod.append(methodDescriptor);
    if (yrt.nativeMethods.find(nativeMethod) != yrt.nativeMethods.end()) {
        return ((*yrt.nativeMethods.find(nativeMethod)).second)(&yrt);
    }

    GC_SAFE_POINT
    if (yrt.gc->shallGC()) {
        yrt.gc->stopTheWorld();
        yrt.gc->gc(GCPolicy::GC_MARK_AND_SWEEP);
    }
    return nullptr;
}