/*
 * VkShaderModule.cpp
 *
 * Vulkan shader module implementation with full SPIR-V reflection.
 */

#include "VkShaderModule.h"

#include <cstring>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

namespace kx {

// ============================================================================
// SPIR-V Opcode constants
// ============================================================================

enum SpvOp : uint16_t {
    SpvOpNop = 0,
    SpvOpUndef = 1,
    SpvOpSourceContinued = 2,
    SpvOpSource = 3,
    SpvOpSourceExtension = 4,
    SpvOpName = 5,
    SpvOpMemberName = 6,
    SpvOpString = 7,
    SpvOpLine = 8,
    SpvOpExtension = 10,
    SpvOpExtInstImport = 11,
    SpvOpExtInst = 12,
    SpvOpMemoryModel = 14,
    SpvOpEntryPoint = 15,
    SpvOpExecutionMode = 16,
    SpvOpCapability = 17,
    SpvOpTypeVoid = 19,
    SpvOpTypeBool = 20,
    SpvOpTypeInt = 21,
    SpvOpTypeFloat = 22,
    SpvOpTypeVector = 23,
    SpvOpTypeMatrix = 24,
    SpvOpTypeImage = 25,
    SpvOpTypeSampler = 26,
    SpvOpTypeSampledImage = 27,
    SpvOpTypeArray = 28,
    SpvOpTypeRuntimeArray = 29,
    SpvOpTypeStruct = 30,
    SpvOpTypeOpaque = 31,
    SpvOpTypePointer = 32,
    SpvOpTypeFunction = 33,
    SpvOpTypeEvent = 34,
    SpvOpTypeDeviceEvent = 35,
    SpvOpTypeReserveId = 36,
    SpvOpTypeQueue = 37,
    SpvOpTypePipe = 38,
    SpvOpTypeForwardPointer = 39,
    // Extension types (using high values to avoid conflicts with core opcodes)
    SpvOpTypeAccelerationStructureKHR = 5000,
    SpvOpTypeRayQueryKHR = 5001,

    SpvOpConstantTrue = 41,
    SpvOpConstantFalse = 42,
    SpvOpConstant = 43,
    SpvOpConstantComposite = 44,
    SpvOpConstantSampler = 45,
    SpvOpConstantNull = 46,
    SpvOpSpecConstantTrue = 48,
    SpvOpSpecConstantFalse = 49,
    SpvOpSpecConstant = 50,
    SpvOpSpecConstantComposite = 51,
    SpvOpSpecConstantOp = 52,
    SpvOpFunction = 54,
    SpvOpFunctionParameter = 55,
    SpvOpFunctionEnd = 56,
    SpvOpFunctionCall = 57,
    SpvOpVariable = 59,
    SpvOpImageTexelPointer = 60,
    SpvOpLoad = 61,
    SpvOpStore = 62,
    SpvOpCopyMemory = 63,
    SpvOpCopyMemorySized = 64,
    SpvOpAccessChain = 65,
    SpvOpInBoundsAccessChain = 66,
    SpvOpPtrAccessChain = 67,
    SpvOpArrayLength = 68,
    SpvOpGenericPtrMemSemantics = 69,
    SpvOpInBoundsPtrAccessChain = 70,
    SpvOpDecorate = 71,
    SpvOpMemberDecorate = 72,
    SpvOpDecorationGroup = 73,
    SpvOpGroupDecorate = 74,
    SpvOpGroupMemberDecorate = 75,
    SpvOpVectorExtractDynamic = 77,
    SpvOpVectorInsertDynamic = 78,
    SpvOpVectorShuffle = 79,
    SpvOpCompositeConstruct = 80,
    SpvOpCompositeExtract = 81,
    SpvOpCompositeInsert = 82,
    SpvOpCopyObject = 83,
    SpvOpTranspose = 84,
    SpvOpSNegate = 86,
    SpvOpFNegate = 87,
    SpvOpIAdd = 88,
    SpvOpFAdd = 89,
    SpvOpISub = 90,
    SpvOpFSub = 91,
    SpvOpIMul = 92,
    SpvOpFMul = 93,
    SpvOpUDiv = 94,
    SpvOpSDiv = 95,
    SpvOpFDiv = 96,
    SpvOpUMod = 97,
    SpvOpSRem = 98,
    SpvOpSMod = 99,
    SpvOpFRem = 100,
    SpvOpFMod = 101,
    SpvOpVectorTimesScalar = 102,
    SpvOpMatrixTimesScalar = 103,
    SpvOpVectorTimesMatrix = 104,
    SpvOpMatrixTimesVector = 105,
    SpvOpMatrixTimesMatrix = 106,
    SpvOpOuterProduct = 107,
    SpvOpDot = 108,
    SpvOpIAddCarry = 109,
    SpvOpISubBorrow = 110,
    SpvOpUMulExtended = 111,
    SpvOpSMulExtended = 112,
    SpvOpAny = 113,
    SpvOpAll = 114,
    SpvOpIsNan = 115,
    SpvOpIsInf = 116,
    SpvOpSignBitSet = 117,
    SpvOpLessOrGreater = 118,
    SpvOpOrdered = 119,
    SpvOpUnordered = 120,
    SpvOpLogicalEqual = 121,
    SpvOpLogicalNotEqual = 122,
    SpvOpLogicalOr = 123,
    SpvOpLogicalAnd = 124,
    SpvOpLogicalNot = 125,
    SpvOpSelect = 126,
    SpvOpIEqual = 127,
    SpvOpINotEqual = 128,
    SpvOpUGreaterThan = 129,
    SpvOpSGreaterThan = 130,
    SpvOpUGreaterThanEqual = 131,
    SpvOpSGreaterThanEqual = 132,
    SpvOpULessThan = 133,
    SpvOpSLessThan = 134,
    SpvOpULessThanEqual = 135,
    SpvOpSLessThanEqual = 136,
    SpvOpFOrdEqual = 137,
    SpvOpFUnordEqual = 138,
    SpvOpFOrdNotEqual = 139,
    SpvOpFUnordNotEqual = 140,
    SpvOpFOrdGreaterThan = 141,
    SpvOpFUnordGreaterThan = 142,
    SpvOpFOrdGreaterThanEqual = 143,
    SpvOpFUnordGreaterThanEqual = 144,
    SpvOpFOrdLessThan = 145,
    SpvOpFUnordLessThan = 146,
    SpvOpFOrdLessThanEqual = 147,
    SpvOpFUnordLessThanEqual = 148,
    SpvOpShiftRightLogical = 149,
    SpvOpShiftRightArithmetic = 150,
    SpvOpShiftLeftLogical = 151,
    SpvOpBitwiseOr = 152,
    SpvOpBitwiseXor = 153,
    SpvOpBitwiseAnd = 154,
    SpvOpNot = 155,
    SpvOpBitFieldInsert = 156,
    SpvOpBitFieldSExtract = 157,
    SpvOpBitFieldUExtract = 158,
    SpvOpBitReverse = 159,
    SpvOpBitCount = 160,
    SpvOpDPdx = 161,
    SpvOpDPdy = 162,
    SpvOpFwidth = 163,
    SpvOpDPdxFine = 164,
    SpvOpDPdyFine = 165,
    SpvOpFwidthFine = 166,
    SpvOpDPdxCoarse = 167,
    SpvOpDPdyCoarse = 168,
    SpvOpFwidthCoarse = 169,
    SpvOpEmitVertex = 170,
    SpvOpEndPrimitive = 171,
    SpvOpEmitStreamVertex = 172,
    SpvOpEndStreamPrimitive = 173,
    SpvOpControlBarrier = 174,
    SpvOpMemoryBarrier = 175,
    SpvOpAtomicLoad = 177,
    SpvOpAtomicStore = 178,
    SpvOpAtomicExchange = 179,
    SpvOpAtomicCompareExchange = 180,
    SpvOpAtomicCompareExchangeWeak = 181,
    SpvOpAtomicIIncrement = 182,
    SpvOpAtomicIDecrement = 183,
    SpvOpAtomicIAdd = 184,
    SpvOpAtomicISub = 185,
    SpvOpAtomicSMin = 186,
    SpvOpAtomicUMin = 187,
    SpvOpAtomicSMax = 188,
    SpvOpAtomicUMax = 189,
    SpvOpAtomicAnd = 190,
    SpvOpAtomicOr = 191,
    SpvOpAtomicXor = 192,
    SpvOpPhi = 193,
    SpvOpLoopMerge = 194,
    SpvOpSelectionMerge = 195,
    SpvOpBranch = 196,
    SpvOpBranchConditional = 197,
    SpvOpSwitch = 198,
    SpvOpKill = 199,
    SpvOpReturn = 200,
    SpvOpReturnValue = 201,
    SpvOpUnreachable = 202,
    SpvOpLifetimeStart = 203,
    SpvOpLifetimeStop = 204,
    SpvOpGroupAsyncCopy = 205,
    SpvOpGroupWaitEvents = 206,
    SpvOpGroupAll = 207,
    SpvOpGroupAny = 208,
    SpvOpGroupBroadcast = 209,
    SpvOpGroupIAdd = 210,
    SpvOpGroupFAdd = 211,
    SpvOpGroupFMin = 212,
    SpvOpGroupUMin = 213,
    SpvOpGroupSMin = 214,
    SpvOpGroupFMax = 215,
    SpvOpGroupUMax = 216,
    SpvOpGroupSMax = 217,
    SpvOpReadPipe = 218,
    SpvOpWritePipe = 219,
    SpvOpReservedReadPipe = 220,
    SpvOpReservedWritePipe = 221,
    SpvOpGroupFMinMax = 222,
    SpvOpGroupUMinMax = 223,
    SpvOpGroupSMinMax = 224,
    SpvOpGroupFMaxMin = 225,
    SpvOpGroupUMaxMin = 226,
    SpvOpGroupSMaxMin = 227,
    SpvOpAtomicFlagTestAndSet = 228,
    SpvOpAtomicFlagClear = 229,
    SpvOpImageSparseSampleImplicitLod = 230,
    SpvOpImageSparseSampleExplicitLod = 231,
    SpvOpImageSparseSampleDrefImplicitLod = 232,
    SpvOpImageSparseSampleDrefExplicitLod = 233,
    SpvOpImageSparseSampleProjImplicitLod = 234,
    SpvOpImageSparseSampleProjExplicitLod = 235,
    SpvOpImageSparseFetch = 236,
    SpvOpImageSparseGather = 237,
    SpvOpImageSparseDrefGather = 238,
    SpvOpImageSparseTexelsResident = 239,
    SpvOpNoLine = 240,
    SpvOpAtomicIncrement = 241,
    SpvOpAtomicDecrement = 242,
    SpvOpSubgroupBallotKHR = 243,
    SpvOpSubgroupFirstInvocationKHR = 244,
    SpvOpSubgroupAllKHR = 245,
    SpvOpSubgroupAnyKHR = 246,
    SpvOpSubgroupAllEqualKHR = 247,
    SpvOpSubgroupReadInvocationKHR = 248,
    SpvOpDecorateId = 249,
    SpvOpDecorateStringGOOGLE = 250,
    SpvOpMemberDecorateStringGOOGLE = 251,
    // ... many more opcodes
};

// Decoration constants
enum SpvDecoration : uint32_t {
    SpvDecorationRelaxedPrecision = 0,
    SpvDecorationSpecId = 1,
    SpvDecorationBlock = 2,
    SpvDecorationBufferBlock = 3,
    SpvDecorationRowMajor = 4,
    SpvDecorationColMajor = 5,
    SpvDecorationArrayStride = 6,
    SpvDecorationMatrixStride = 7,
    SpvDecorationGLSLShared = 8,
    SpvDecorationGLSLPacked = 9,
    SpvDecorationBuiltIn = 10,
    SpvDecorationNoPerspective = 11,
    SpvDecorationFlat = 12,
    SpvDecorationPatch = 13,
    SpvDecorationCentroid = 14,
    SpvDecorationSample = 15,
    SpvDecorationInvariant = 16,
    SpvDecorationRestrict = 17,
    SpvDecorationAliased = 18,
    SpvDecorationVolatile = 19,
    SpvDecorationConstant = 20,
    SpvDecorationCoherent = 21,
    SpvDecorationNonWritable = 22,
    SpvDecorationNonReadable = 23,
    SpvDecorationUniform = 24,
    SpvDecorationUniformId = 25,
    SpvDecorationSaturatedConversion = 26,
    SpvDecorationStream = 27,
    SpvDecorationLocation = 30,
    SpvDecorationComponent = 31,
    SpvDecorationIndex = 32,
    SpvDecorationBinding = 33,
    SpvDecorationDescriptorSet = 34,
    SpvDecorationOffset = 35,
    SpvDecorationXfbBuffer = 36,
    SpvDecorationXfbStride = 37,
    SpvDecorationFuncParamAttr = 38,
    SpvDecorationFPRoundingMode = 39,
    SpvDecorationFPFastMathMode = 40,
    SpvDecorationLinkageAttributes = 41,
    SpvDecorationNoContraction = 42,
    SpvDecorationInputAttachmentIndex = 43,
    SpvDecorationAlignment = 44,
    SpvDecorationMaxByteOffset = 45,
    SpvDecorationAlignmentId = 46,
    SpvDecorationMaxByteOffsetId = 47,
    SpvDecorationNoSignedWrap = 48,
    SpvDecorationNoUnsignedWrap = 49,
    SpvDecorationExplicitInterpAMD = 50,
    SpvDecorationOverrideCoverageNV = 52,
    SpvDecorationPassthroughNV = 53,
    SpvDecorationSpecIdId = 54,
    SpvDecorationNonUniform = 55,
    SpvDecorationRestrictPointer = 56,
    SpvDecorationAliasedPointer = 57,
    SpvDecorationConstantPointer = 58,
    SpvDecorationStorageClass = 59,
    SpvDecorationSaturatedMath = 60,
    SpvDecorationExplicitInterp = 61,
    SpvDecorationNoDuplicate = 62,
    SpvDecorationNoAlias = 63,
    SpvDecorationRestrictQualifier = 64,
    SpvDecorationAliasedQualifier = 65,
    SpvDecorationVolatileQualifier = 66,
    SpvDecorationConstantQualifier = 67,
    SpvDecorationPipeStorage = 68,
    SpvDecorationCoherentQualifier = 69,
    SpvDecorationUniformQualifier = 70,
    SpvDecorationNonWritableQualifier = 71,
    SpvDecorationNonReadableQualifier = 72,
    SpvDecorationUniformIdKHR = 73,
    SpvDecorationMaxVertexCount = 74,
    SpvDecorationPrimitiveTopology = 75,
    SpvDecorationEarlyFragmentTests = 76,
    SpvDecorationPostDepthCoverage = 77,
    SpvDecorationPixelInterlockOrderedEXT = 78,
    SpvDecorationPixelInterlockUnorderedEXT = 79,
    SpvDecorationSampleInterlockOrderedEXT = 80,
    SpvDecorationSampleInterlockUnorderedEXT = 81,
    SpvDecorationLayout = 82,
    SpvDecorationLinkageArrayStride = 83,
    SpvDecorationNoStaticUse = 84,
    SpvDecorationShaderRecordBufferKHR = 85,
    SpvDecorationRayFlagsKHR = 86,
    SpvDecorationCullMaskKHR = 87,
    SpvDecorationPrimitiveIdKHR = 88,
    SpvDecorationInstanceIdKHR = 89,
    SpvDecorationGeometryIdKHR = 90,
    SpvDecorationLayered = 91,
    SpvDecorationCullableKHR = 92,
    SpvDecorationDepthReplacingKHR = 93,
    SpvDecorationLaunchIdKHR = 94,
    SpvDecorationLaunchSizeKHR = 95,
    SpvDecorationWorldRayOriginKHR = 96,
    SpvDecorationWorldRayDirectionKHR = 97,
    SpvDecorationObjectRayOriginKHR = 98,
    SpvDecorationObjectRayDirectionKHR = 99,
    SpvDecorationRayTminKHR = 100,
    SpvDecorationRayTmaxKHR = 101,
    SpvDecorationInstanceCustomIndexKHR = 102,
    SpvDecorationObjectToWorldKHR = 103,
    SpvDecorationWorldToObjectKHR = 104,
    SpvDecorationHitT = 105,
    SpvDecorationHitKindKHR = 106,
    SpvDecorationIncomingRayFlagsKHR = 107,
    SpvDecorationStackSizeKHR = 108,
};

// Storage Class constants
enum SpvStorageClass : uint32_t {
    SpvStorageClassUniformConstant = 0,
    SpvStorageClassInput = 1,
    SpvStorageClassUniform = 2,
    SpvStorageClassOutput = 3,
    SpvStorageClassWorkgroup = 4,
    SpvStorageClassCrossWorkgroup = 5,
    SpvStorageClassPrivate = 6,
    SpvStorageClassFunction = 7,
    SpvStorageClassGeneric = 8,
    SpvStorageClassPushConstant = 9,
    SpvStorageClassAtomicCounter = 10,
    SpvStorageClassImage = 11,
    SpvStorageClassStorageBuffer = 12,
    SpvStorageClassCallableDataKHR = 13,
    SpvStorageClassIncomingCallableDataKHR = 14,
    SpvStorageClassRayPayloadKHR = 15,
    SpvStorageClassHitAttributeKHR = 16,
    SpvStorageClassIncomingRayPayloadKHR = 17,
    SpvStorageClassShaderRecordBufferKHR = 18,
    SpvStorageClassPhysicalStorageBuffer = 19,
    SpvStorageClassCodeSectionINTEL = 20,
};

// BuiltIn constants (subset)
enum SpvBuiltIn : uint32_t {
    SpvBuiltInPosition = 0,
    SpvBuiltInPointSize = 1,
    SpvBuiltInClipVertex = 2,
    SpvBuiltInClipDistance = 3,
    SpvBuiltInCullDistance = 4,
    SpvBuiltInVertexId = 5,
    SpvBuiltInInstanceId = 6,
    SpvBuiltInPrimitiveId = 7,
    SpvBuiltInInvocationId = 8,
    SpvBuiltInLayer = 9,
    SpvBuiltInViewportIndex = 10,
    SpvBuiltInTessLevelOuter = 11,
    SpvBuiltInTessLevelInner = 12,
    SpvBuiltInTessCoord = 13,
    SpvBuiltInPatchVertices = 14,
    SpvBuiltInFragCoord = 15,
    SpvBuiltInPointCoord = 16,
    SpvBuiltInFrontFacing = 17,
    SpvBuiltInSampleId = 18,
    SpvBuiltInSamplePosition = 19,
    SpvBuiltInSampleMask = 20,
    SpvBuiltInFragDepth = 22,
    SpvBuiltInHelperInvocation = 23,
    SpvBuiltInNumWorkgroups = 24,
    SpvBuiltInWorkgroupSize = 25,
    SpvBuiltInWorkgroupId = 26,
    SpvBuiltInLocalInvocationId = 27,
    SpvBuiltInGlobalInvocationId = 28,
    SpvBuiltInLocalInvocationIndex = 29,
    SpvBuiltInWorkDim = 30,
    SpvBuiltInGlobalSize = 31,
    SpvBuiltInEnqueuedWorkgroupSize = 32,
    SpvBuiltInGlobalOffset = 33,
    SpvBuiltInGlobalLinearId = 34,
    SpvBuiltInSubgroupSize = 36,
    SpvBuiltInSubgroupMaxSize = 37,
    SpvBuiltInNumSubgroups = 38,
    SpvBuiltInNumEnqueuedSubgroups = 39,
    SpvBuiltInSubgroupId = 40,
    SpvBuiltInSubgroupLocalInvocationId = 41,
};

// ============================================================================
// SPIR-V Reflection Data Structures
// ============================================================================

struct SpirvVariable {
    uint32_t resultId = 0;
    uint32_t typeId = 0;
    uint32_t storageClass = 0;
    std::string name;
    
    // Decorations
    uint32_t descriptorSet = 0;
    uint32_t binding = 0;
    uint32_t location = 0;
    uint32_t offset = 0;
    uint32_t builtIn = 0;
    bool hasDescriptorSet = false;
    bool hasBinding = false;
    bool hasLocation = false;
    bool hasOffset = false;
    bool hasBuiltIn = false;
};

struct SpirvType {
    uint32_t resultId = 0;
    uint32_t opcode = 0;  // SpvOpType*
    std::vector<uint32_t> operands;
    
    // For composite types
    uint32_t elementTypeId = 0;  // For arrays, vectors, matrices, pointers
    uint32_t elementCount = 0;   // For arrays, vectors, matrices
    uint32_t baseTypeId = 0;     // For pointers
    uint32_t storageClass = 0;   // For pointers
    
    // For structs
    std::vector<uint32_t> memberTypeIds;
    
    // For images
    uint32_t dim = 0;
    uint32_t depth = 0;
    uint32_t arrayed = 0;
    uint32_t ms = 0;
    uint32_t sampled = 0;
    uint32_t imageFormat = 0;
    uint32_t accessQualifier = 0;
};

struct SpirvConstant {
    uint32_t resultId = 0;
    uint32_t typeId = 0;
    std::vector<uint32_t> words;
};

struct SpirvInterfaceVariable {
    uint32_t varId = 0;
    uint32_t location = 0;
    uint32_t component = 0;
    uint32_t builtIn = 0;
    bool hasLocation = false;
    bool hasComponent = false;
    bool hasBuiltIn = false;
};

struct SpirvExecutionMode {
    uint32_t entryPointId = 0;
    uint32_t mode = 0;
    std::vector<uint32_t> operands;
};

// ============================================================================
// SPIR-V Parser
// ============================================================================

class SpirvParser {
public:
    explicit SpirvParser(const std::vector<uint32_t>& spirv)
        : words_(spirv) {}
    
    void parse() {
        if (words_.size() < 5) return;
        if (words_[0] != 0x07230203) return;  // Magic number
        
        uint32_t wordCount = words_[1];
        if (words_.size() < wordCount) return;
        
        uint32_t bound = words_[3];
        
        // Resize lookup tables
        variables_.resize(bound);
        types_.resize(bound);
        constants_.resize(bound);
        names_.resize(bound);
        memberNames_.resize(bound);
        decorations_.resize(bound);
        executionModes_.clear();
        interfaceVariables_.clear();
        
        // Parse all instructions
        for (uint32_t i = 5; i < wordCount; ) {
            if (i >= words_.size()) break;
            
            uint32_t word = words_[i];
            uint16_t opcode = static_cast<uint16_t>(word & 0xFFFF);
            uint16_t wordCount_instr = static_cast<uint16_t>(word >> 16);
            
            if (i + wordCount_instr > wordCount) break;
            
            parseInstruction(opcode, i, wordCount_instr);
            
            i += wordCount_instr;
        }
    }
    
    // Get reflection results
    const std::vector<SpirvVariable>& getVariables() const { return variables_; }
    const std::vector<SpirvType>& getTypes() const { return types_; }
    const std::vector<std::string>& getNames() const { return names_; }
    const std::vector<SpirvExecutionMode>& getExecutionModes() const { return executionModes_; }
    const std::vector<SpirvInterfaceVariable>& getInterfaceVariables() const { return interfaceVariables_; }
    
    // Find entry point
    uint32_t getEntryPointId() const { return entryPointId_; }
    uint32_t getEntryPointStage() const { return entryPointStage_; }
    const std::string& getEntryPointName() const { return entryPointName_; }
    
private:
    const std::vector<uint32_t>& words_;
    
    // Lookup tables indexed by result ID
    std::vector<SpirvVariable> variables_;
    std::vector<SpirvType> types_;
    std::vector<SpirvConstant> constants_;
    std::vector<std::string> names_;
    std::vector<std::vector<std::string>> memberNames_;  // [id][member_index]
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> decorations_;  // [id] -> list of (decoration, value)
    
    // Entry point info
    uint32_t entryPointId_ = 0;
    uint32_t entryPointStage_ = 0;
    std::string entryPointName_;
    
    // Execution modes
    std::vector<SpirvExecutionMode> executionModes_;
    
    // Interface variables (from entry point)
    std::vector<SpirvInterfaceVariable> interfaceVariables_;
    
    void parseInstruction(uint16_t opcode, uint32_t index, uint16_t wordCount) {
        switch (opcode) {
            case SpvOpEntryPoint:
                parseEntryPoint(index, wordCount);
                break;
            case SpvOpExecutionMode:
                parseExecutionMode(index, wordCount);
                break;
            case SpvOpVariable:
                parseVariable(index, wordCount);
                break;
            case SpvOpTypeVoid:
            case SpvOpTypeBool:
            case SpvOpTypeInt:
            case SpvOpTypeFloat:
            case SpvOpTypeVector:
            case SpvOpTypeMatrix:
            case SpvOpTypeImage:
            case SpvOpTypeSampler:
            case SpvOpTypeSampledImage:
            case SpvOpTypeArray:
            case SpvOpTypeRuntimeArray:
            case SpvOpTypeStruct:
            case SpvOpTypeOpaque:
            case SpvOpTypePointer:
            case SpvOpTypeFunction:
            case SpvOpTypeEvent:
            case SpvOpTypeDeviceEvent:
            case SpvOpTypeQueue:
            case SpvOpTypePipe:
                parseType(opcode, index, wordCount);
                break;
            case SpvOpConstant:
            case SpvOpConstantTrue:
            case SpvOpConstantFalse:
            case SpvOpConstantComposite:
            case SpvOpConstantSampler:
            case SpvOpConstantNull:
            case SpvOpSpecConstant:
            case SpvOpSpecConstantTrue:
            case SpvOpSpecConstantFalse:
            case SpvOpSpecConstantComposite:
            case SpvOpSpecConstantOp:
                parseConstant(opcode, index, wordCount);
                break;
            case SpvOpDecorate:
                parseDecorate(index, wordCount);
                break;
            case SpvOpMemberDecorate:
                parseMemberDecorate(index, wordCount);
                break;
            case SpvOpName:
                parseName(index, wordCount);
                break;
            case SpvOpMemberName:
                parseMemberName(index, wordCount);
                break;
            case SpvOpString:
                parseString(index, wordCount);
                break;
            default:
                break;
        }
    }
    
    void parseEntryPoint(uint32_t index, uint16_t wordCount) {
        if (wordCount < 4) return;
        
        entryPointStage_ = words_[index + 1];
        entryPointId_ = words_[index + 2];
        
        // Entry point name starts at index + 3
        const char* nameStart = reinterpret_cast<const char*>(&words_[index + 3]);
        entryPointName_ = nameStart;
        
        // Interface variables follow the name (null-terminated string)
        // Find end of string
        uint32_t nameEnd = index + 3;
        while (nameEnd < words_.size() && words_[nameEnd] != 0) {
            nameEnd++;
        }
        // Round up to next word boundary
        nameEnd = (nameEnd + 3) & ~3;
        
        // Remaining words are interface variable IDs
        for (uint32_t i = nameEnd; i < index + wordCount; ++i) {
            uint32_t varId = words_[i];
            if (varId < variables_.size() && variables_[varId].resultId != 0) {
                SpirvInterfaceVariable ivar;
                ivar.varId = varId;
                
                // Get decorations for this variable
                for (const auto& dec : decorations_[varId]) {
                    if (dec.first == SpvDecorationLocation) {
                        ivar.location = dec.second;
                        ivar.hasLocation = true;
                    } else if (dec.first == SpvDecorationComponent) {
                        ivar.component = dec.second;
                        ivar.hasComponent = true;
                    } else if (dec.first == SpvDecorationBuiltIn) {
                        ivar.builtIn = dec.second;
                        ivar.hasBuiltIn = true;
                    }
                }
                interfaceVariables_.push_back(ivar);
            }
        }
    }
    
    void parseExecutionMode(uint32_t index, uint16_t wordCount) {
        if (wordCount < 3) return;
        
        SpirvExecutionMode em;
        em.entryPointId = words_[index + 1];
        em.mode = words_[index + 2];
        
        for (uint32_t i = 3; i < wordCount; ++i) {
            em.operands.push_back(words_[index + i]);
        }
        executionModes_.push_back(em);
    }
    
    void parseVariable(uint32_t index, uint16_t wordCount) {
        // OpVariable: ResultType, ResultId, StorageClass, [Initializer]
        if (wordCount < 4) return;
        
        uint32_t resultType = words_[index + 1];
        uint32_t resultId = words_[index + 2];
        uint32_t storageClass = words_[index + 3];
        
        if (resultId < variables_.size()) {
            SpirvVariable& var = variables_[resultId];
            var.resultId = resultId;
            var.typeId = resultType;
            var.storageClass = storageClass;
            
            // Apply any existing decorations
            applyDecorationsToVariable(var);
        }
    }
    
    void parseType(uint16_t opcode, uint32_t index, uint16_t wordCount) {
        if (wordCount < 3) return;
        
        uint32_t resultId = words_[index + 2];
        if (resultId >= types_.size()) return;
        
        SpirvType& type = types_[resultId];
        type.resultId = resultId;
        type.opcode = opcode;
        
        switch (opcode) {
            case SpvOpTypeVector:
                if (wordCount >= 4) {
                    type.elementTypeId = words_[index + 3];
                    type.elementCount = words_[index + 4];
                }
                break;
            case SpvOpTypeMatrix:
                if (wordCount >= 4) {
                    type.elementTypeId = words_[index + 3];
                    type.elementCount = words_[index + 4];
                }
                break;
            case SpvOpTypeArray:
                if (wordCount >= 4) {
                    type.elementTypeId = words_[index + 3];
                    type.elementCount = words_[index + 4];  // Could be a constant ID
                }
                break;
            case SpvOpTypeRuntimeArray:
                if (wordCount >= 4) {
                    type.elementTypeId = words_[index + 3];
                }
                break;
            case SpvOpTypeStruct:
                for (uint32_t i = 3; i < wordCount; ++i) {
                    type.memberTypeIds.push_back(words_[index + i]);
                }
                break;
            case SpvOpTypeImage:
                if (wordCount >= 9) {
                    type.elementTypeId = words_[index + 3];
                    type.dim = words_[index + 4];
                    type.depth = words_[index + 5];
                    type.arrayed = words_[index + 6];
                    type.ms = words_[index + 7];
                    type.sampled = words_[index + 8];
                    type.imageFormat = words_[index + 9];
                }
                break;
            case SpvOpTypeSampler:
                break;
            case SpvOpTypeSampledImage:
                if (wordCount >= 4) {
                    type.elementTypeId = words_[index + 3];
                }
                break;
            case SpvOpTypePointer:
                if (wordCount >= 4) {
                    type.storageClass = words_[index + 3];
                    type.baseTypeId = words_[index + 4];
                }
                break;
            case SpvOpTypeFunction:
                if (wordCount >= 4) {
                    type.elementTypeId = words_[index + 3];  // Return type
                    for (uint32_t i = 4; i < wordCount; ++i) {
                        type.memberTypeIds.push_back(words_[index + i]);  // Parameter types
                    }
                }
                break;
            default:
                break;
        }
    }
    
    void parseConstant(uint16_t opcode, uint32_t index, uint16_t wordCount) {
        if (wordCount < 4) return;
        
        uint32_t resultId = words_[index + 2];
        if (resultId >= constants_.size()) return;
        
        SpirvConstant& c = constants_[resultId];
        c.resultId = resultId;
        c.typeId = words_[index + 1];
        
        for (uint32_t i = 3; i < wordCount; ++i) {
            c.words.push_back(words_[index + i]);
        }
    }
    
    void parseDecorate(uint32_t index, uint16_t wordCount) {
        // OpDecorate: Target, Decoration, [Params...]
        if (wordCount < 3) return;
        
        uint32_t target = words_[index + 1];
        uint32_t decoration = words_[index + 2];
        
        if (target < decorations_.size()) {
            decorations_[target].emplace_back(decoration, wordCount > 3 ? words_[index + 3] : 0);
            
            // Apply to variable if it exists
            if (target < variables_.size() && variables_[target].resultId != 0) {
                applyDecorationToVariable(variables_[target], decoration, wordCount > 3 ? words_[index + 3] : 0);
            }
        }
    }
    
    void parseMemberDecorate(uint32_t index, uint16_t wordCount) {
        // OpMemberDecorate: StructureType, Member, Decoration, [Params...]
        if (wordCount < 4) return;
        // Could implement for struct member offsets
    }
    
    void parseName(uint32_t index, uint16_t wordCount) {
        if (wordCount < 3) return;
        
        uint32_t target = words_[index + 1];
        const char* nameStart = reinterpret_cast<const char*>(&words_[index + 2]);
        
        if (target < names_.size()) {
            names_[target] = nameStart;
        }
    }
    
    void parseMemberName(uint32_t index, uint16_t wordCount) {
        if (wordCount < 4) return;
        
        uint32_t typeId = words_[index + 1];
        uint32_t memberIndex = words_[index + 2];
        const char* nameStart = reinterpret_cast<const char*>(&words_[index + 3]);
        
        if (typeId < memberNames_.size()) {
            if (memberIndex >= memberNames_[typeId].size()) {
                memberNames_[typeId].resize(memberIndex + 1);
            }
            memberNames_[typeId][memberIndex] = nameStart;
        }
    }
    
    void parseString(uint32_t index, uint16_t wordCount) {
        // OpString is just a string literal, usually referenced by OpName/OpMemberName
    }
    
    void applyDecorationsToVariable(SpirvVariable& var) {
        if (var.resultId >= decorations_.size()) return;
        
        for (const auto& dec : decorations_[var.resultId]) {
            applyDecorationToVariable(var, dec.first, dec.second);
        }
    }
    
    void applyDecorationToVariable(SpirvVariable& var, uint32_t decoration, uint32_t value) {
        switch (decoration) {
            case SpvDecorationDescriptorSet:
                var.descriptorSet = value;
                var.hasDescriptorSet = true;
                break;
            case SpvDecorationBinding:
                var.binding = value;
                var.hasBinding = true;
                break;
            case SpvDecorationLocation:
                var.location = value;
                var.hasLocation = true;
                break;
            case SpvDecorationOffset:
                var.offset = value;
                var.hasOffset = true;
                break;
            case SpvDecorationBuiltIn:
                var.builtIn = value;
                var.hasBuiltIn = true;
                break;
            default:
                break;
        }
    }
};
 
// ============================================================================
// Helper Functions for SPIR-V Reflection
// ============================================================================
 
// Calculate size of a type in bytes
static uint32_t calculateTypeSize(const std::vector<SpirvType>& types, uint32_t typeId) {
    if (typeId >= types.size()) return 0;
    const SpirvType& type = types[typeId];
    
    switch (type.opcode) {
        case SpvOpTypeInt:
        case SpvOpTypeFloat:
            // Width is in operand (bits), divide by 8 for bytes
            if (type.operands.size() >= 1) {
                return type.operands[0] / 8;
            }
            return 4;
        case SpvOpTypeVector:
            return calculateTypeSize(types, type.elementTypeId) * type.elementCount;
        case SpvOpTypeMatrix:
            // Matrix is array of column vectors
            return calculateTypeSize(types, type.elementTypeId) * type.elementCount;
        case SpvOpTypeArray:
            if (type.elementCount > 0) {
                return calculateTypeSize(types, type.elementTypeId) * type.elementCount;
            }
            return 0;  // Runtime array
        case SpvOpTypeRuntimeArray:
            return 0;  // Unsized
        case SpvOpTypeStruct: {
            uint32_t totalSize = 0;
            for (uint32_t memberTypeId : type.memberTypeIds) {
                totalSize += calculateTypeSize(types, memberTypeId);
            }
            // TODO: Account for padding/alignment using decorations
            return totalSize;
        }
        case SpvOpTypeImage:
        case SpvOpTypeSampler:
        case SpvOpTypeSampledImage:
            return 0;  // Opaque types, no size
        default:
            return 0;
    }
}
 
// Get VkDescriptorType from SPIR-V storage class and type
static VkDescriptorType getDescriptorTypeFromStorageClassAndType(
    SpvStorageClass storageClass,
    const std::vector<SpirvType>& types,
    uint32_t typeId) {
    
    if (typeId >= types.size()) return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    
    const SpirvType& type = types[typeId];
    
    switch (storageClass) {
        case SpvStorageClassUniformConstant:
            // Could be sampler, sampled image, storage image, or acceleration structure
            switch (type.opcode) {
                case SpvOpTypeSampler:
                    return VK_DESCRIPTOR_TYPE_SAMPLER;
                case SpvOpTypeSampledImage:
                    return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                case SpvOpTypeImage:
                    // Check if storage image (read/write access)
                    // Check access qualifier from decorations if available
                    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                case SpvOpTypeAccelerationStructureKHR:
                    return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                default:
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            }
        case SpvStorageClassStorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case SpvStorageClassUniform:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case SpvStorageClassPushConstant:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;  // Not a descriptor
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}
 
// ============================================================================
// VkShaderModuleImpl Implementation
// ============================================================================
 
VkShaderModuleImpl::~VkShaderModuleImpl() {
    if (module_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkShaderModuleImpl::VkShaderModuleImpl(VkShaderModuleImpl&& other) noexcept
    : module_(other.module_)
    , device_(other.device_)
    , spirv_(std::move(other.spirv_))
    , stage_(other.stage_)
    , entry_point_(std::move(other.entry_point_))
    , bindings_(std::move(other.bindings_))
    , push_constants_(std::move(other.push_constants_)) {
    other.module_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.stage_ = 0;
}

VkShaderModuleImpl& VkShaderModuleImpl::operator=(VkShaderModuleImpl&& other) noexcept {
    if (this != &other) {
        if (module_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
            // Note: Can't destroy without function table
        }
        module_ = other.module_;
        device_ = other.device_;
        spirv_ = std::move(other.spirv_);
        stage_ = other.stage_;
        entry_point_ = std::move(other.entry_point_);
        bindings_ = std::move(other.bindings_);
        push_constants_ = std::move(other.push_constants_);
        other.module_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.stage_ = 0;
    }
    return *this;
}

bool VkShaderModuleImpl::initialize(VkFunctionTable& functions,
                                    VkDevice device,
                                    const std::vector<uint32_t>& spirv) {
    device_ = device;
    spirv_ = spirv;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = static_cast<size_t>(spirv.size()) * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    VkResult result = functions.vkCreateShaderModule(device, &createInfo, nullptr, &module_);
    if (result != VK_SUCCESS) {
        return false;
    }

    // Reflect the SPIR-V to get bindings and push constants
    reflect_spirv(functions);

    return true;
}

void VkShaderModuleImpl::reflect_spirv(VkFunctionTable& functions) {
    (void)functions;  // Not needed for reflection
    
    // Parse SPIR-V
    SpirvParser parser(spirv_);
    parser.parse();
    
    // Get entry point info
    entry_point_ = parser.getEntryPointName();
    stage_ = parser.getEntryPointStage();
    
    // Get names
    const auto& names = parser.getNames();
    const auto& variables = parser.getVariables();
    const auto& types = parser.getTypes();
    const auto& interfaceVars = parser.getInterfaceVariables();
    
    // Map variable ID -> variable info (with decorations applied)
    std::unordered_map<uint32_t, SpirvVariable> varMap;
    for (const auto& var : variables) {
        if (var.resultId != 0) {
            varMap[var.resultId] = var;
        }
    }
    
    // Process interface variables to find descriptor bindings and push constants
    for (const auto& ivar : interfaceVars) {
        if (ivar.varId >= variables.size()) continue;
        const SpirvVariable& var = variables[ivar.varId];
        
        if (var.resultId == 0) continue;
        
        // Get the type to determine descriptor type
        if (var.typeId >= types.size()) continue;
        const SpirvType& type = types[var.typeId];
        
        // Follow pointer chain to get actual type
        uint32_t actualTypeId = var.typeId;
        SpvStorageClass storageClass = static_cast<SpvStorageClass>(var.storageClass);
        
        // Handle pointer type
        if (type.opcode == SpvOpTypePointer) {
            storageClass = static_cast<SpvStorageClass>(type.storageClass);
            actualTypeId = type.baseTypeId;
        }
        
        // Process based on storage class
        if (storageClass == SpvStorageClassUniformConstant ||
            storageClass == SpvStorageClassStorageBuffer ||
            storageClass == SpvStorageClassUniform ||
            storageClass == SpvStorageClassPushConstant) {
            
            if (storageClass == SpvStorageClassPushConstant) {
                // Push constant
                ShaderReflectionPushConstant pc;
                pc.stageFlags = stage_;
                pc.offset = var.hasOffset ? var.offset : 0;
                
                // Calculate size from type
                pc.size = calculateTypeSize(types, actualTypeId);
                pc.name = varMap.count(var.resultId) && !names[var.resultId].empty() 
                    ? names[var.resultId] 
                    : ("push_constant_" + std::to_string(var.resultId));
                
                push_constants_.push_back(pc);
            } else {
                // Descriptor binding (uniform, storage buffer, sampled image, etc.)
                if (!var.hasDescriptorSet || !var.hasBinding) continue;
                
                ShaderReflectionBinding binding;
                binding.set = var.descriptorSet;
                binding.binding = var.binding;
                binding.stageFlags = stage_;
                binding.count = 1;  // Default, could be array
                binding.name = varMap.count(var.resultId) && !names[var.resultId].empty() 
                    ? names[var.resultId] 
                    : ("binding_" + std::to_string(var.binding));
                
                // Determine descriptor type from storage class and type
                binding.type = getDescriptorTypeFromStorageClassAndType(storageClass, types, actualTypeId);
                
                bindings_.push_back(binding);
            }
        }
    }
    
    // Also check all variables (not just interface) for descriptor bindings
    // Some variables might not be in the entry point interface but still be accessible
    for (const auto& [varId, var] : varMap) {
        if (var.storageClass == SpvStorageClassUniformConstant ||
            var.storageClass == SpvStorageClassStorageBuffer ||
            var.storageClass == SpvStorageClassUniform) {
            
            // Skip if already processed
            bool alreadyProcessed = false;
            for (const auto& b : bindings_) {
                if (b.set == var.descriptorSet && b.binding == var.binding) {
                    alreadyProcessed = true;
                    break;
                }
            }
            if (alreadyProcessed) continue;
            
            if (!var.hasDescriptorSet || !var.hasBinding) continue;
            
            if (var.typeId >= types.size()) continue;
            const SpirvType& type = types[var.typeId];
            
            uint32_t actualTypeId = var.typeId;
            SpvStorageClass storageClass = static_cast<SpvStorageClass>(var.storageClass);
            
            if (type.opcode == SpvOpTypePointer) {
                storageClass = static_cast<SpvStorageClass>(type.storageClass);
                actualTypeId = type.baseTypeId;
            }
            
            ShaderReflectionBinding binding;
            binding.set = var.descriptorSet;
            binding.binding = var.binding;
            binding.stageFlags = stage_;
            binding.count = 1;
            binding.name = !names[varId].empty() ? names[varId] : ("binding_" + std::to_string(var.binding));
            binding.type = getDescriptorTypeFromStorageClassAndType(storageClass, types, actualTypeId);
            
            bindings_.push_back(binding);
        }
    }
    
    // Sort bindings by set, then binding for consistency
    std::sort(bindings_.begin(), bindings_.end(), 
        [](const ShaderReflectionBinding& a, const ShaderReflectionBinding& b) {
            if (a.set != b.set) return a.set < b.set;
            return a.binding < b.binding;
        });
    
    // Sort push constants by offset
    std::sort(push_constants_.begin(), push_constants_.end(),
        [](const ShaderReflectionPushConstant& a, const ShaderReflectionPushConstant& b) {
            return a.offset < b.offset;
        });
}
 
} // namespace kx