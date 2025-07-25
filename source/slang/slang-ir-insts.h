// slang-ir-insts.h
#pragma once

// This file extends the core definitions in `ir.h`
// with a wider variety of concrete instructions,
// and a "builder" abstraction.
//
// TODO: the builder probably needs its own file.

#include "slang-ast-support-types.h"
#include "slang-capability.h"
#include "slang-compiler.h"
#include "slang-ir.h"
#include "slang-syntax.h"
#include "slang-type-layout.h"

//
#include "slang-ir-insts.h.fiddle"

FIDDLE()
namespace Slang
{

class Decl;

FIDDLE()
struct IRCapabilitySet : IRInst
{
    FIDDLE(baseInst())

    CapabilitySet getCaps();
};

FIDDLE()
struct IRDecoration : IRInst
{
    FIDDLE(baseInst())

    IRDecoration* getNextDecoration() { return as<IRDecoration>(getNextInst()); }
};

// Associates an IR-level decoration with a source declaration
// in the high-level AST, that can be used to extract
// additional information that informs code emission.
FIDDLE()
struct IRHighLevelDeclDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRPtrLit* getDeclOperand() { return cast<IRPtrLit>(getOperand(0)); }
    Decl* getDecl() { return (Decl*)getDeclOperand()->getValue(); }
};

enum IRLoopControl
{
    kIRLoopControl_Unroll,
    kIRLoopControl_Loop,
};

FIDDLE()
struct IRLoopControlDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getModeOperand() { return cast<IRConstant>(getOperand(0)); }

    IRLoopControl getMode() { return IRLoopControl(getModeOperand()->value.intVal); }
};

FIDDLE()
struct IRLoopMaxItersDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getMaxItersInst() { return cast<IRConstant>(getOperand(0)); }
    IRIntegerValue getMaxIters() { return as<IRIntLit>(getOperand(0))->getValue(); }
};

FIDDLE()
struct IRTargetSpecificDecoration : IRDecoration
{
    FIDDLE(baseInst())

    IRCapabilitySet* getTargetCapsOperand() { return cast<IRCapabilitySet>(getOperand(0)); }

    CapabilitySet getTargetCaps() { return getTargetCapsOperand()->getCaps(); }

    bool hasPredicate() { return getOperandCount() >= 4; }

    UnownedStringSlice getTypePredicate()
    {
        SLANG_ASSERT(getOperandCount() == 4);
        const auto lit = as<IRStringLit>(getOperand(2));
        SLANG_ASSERT(lit);
        return lit->getStringSlice();
    }

    IRType* getTypeScrutinee()
    {
        SLANG_ASSERT(getOperandCount() == 4);
        // Note: cannot use as<IRType> here because the operand can be
        // an `IRParam` representing a generic type.
        const auto t = (IRType*)(getOperand(3));
        SLANG_ASSERT(t);
        return t;
    }
};

FIDDLE()
struct IRTargetSpecificDefinitionDecoration : IRTargetSpecificDecoration
{
    FIDDLE(baseInst())
};

FIDDLE()
struct IRTargetDecoration : IRTargetSpecificDefinitionDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRTargetSystemValueDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getSemanticOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getSemantic() { return getSemanticOperand()->getStringSlice(); }
};

FIDDLE()
struct IRTargetIntrinsicDecoration : IRTargetSpecificDefinitionDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getDefinitionOperand() { return cast<IRStringLit>(getOperand(1)); }

    UnownedStringSlice getDefinition() { return getDefinitionOperand()->getStringSlice(); }
};

FIDDLE()
struct IRRequirePreludeDecoration : IRTargetSpecificDecoration
{
    FIDDLE(leafInst())

    UnownedStringSlice getPrelude() { return as<IRStringLit>(getOperand(1))->getStringSlice(); }
};

FIDDLE()
struct IRIntrinsicOpDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getIntrinsicOpOperand() { return cast<IRIntLit>(getOperand(0)); }

    IROp getIntrinsicOp() { return (IROp)getIntrinsicOpOperand()->getValue(); }
};

FIDDLE()
struct IRAlignedAddressDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRInst* getAlignment() { return getOperand(0); }
};

FIDDLE()
struct IRGLSLOuterArrayDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getOuterArrayNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getOuterArrayName() { return getOuterArrayNameOperand()->getStringSlice(); }
};

enum class IRInterpolationMode
{
    Linear,
    NoPerspective,
    NoInterpolation,

    Centroid,
    Sample,

    PerVertex,
};

enum class IRTargetBuiltinVarName
{
    Unknown,
    HlslInstanceID,
    HlslVertexID,
    SpvInstanceIndex,
    SpvBaseInstance,
    SpvVertexIndex,
    SpvBaseVertex,
};

FIDDLE()
struct IRInterpolationModeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getModeOperand() { return cast<IRConstant>(getOperand(0)); }

    IRInterpolationMode getMode() { return IRInterpolationMode(getModeOperand()->value.intVal); }
};

/// A decoration that provides a desired name to be used
/// in conjunction with the given instruction. Back-end
/// code generation may use this to help derive symbol
/// names, emit debug information, etc.
FIDDLE()
struct IRNameHintDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getName() { return getNameOperand()->getStringSlice(); }
};

/// A decoration on a RTTIObject providing type size information.
FIDDLE()
struct IRRTTITypeSizeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getTypeSizeOperand() { return cast<IRIntLit>(getOperand(0)); }
    IRIntegerValue getTypeSize() { return getTypeSizeOperand()->getValue(); }
};

/// A decoration on `IRInterfaceType` that marks the size of `AnyValue` that should
/// be used to represent a polymorphic value of the interface.
FIDDLE()
struct IRAnyValueSizeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getSizeOperand() { return cast<IRIntLit>(getOperand(0)); }
    IRIntegerValue getSize() { return getSizeOperand()->getValue(); }
};

FIDDLE()
struct IRDispatchFuncDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getFunc() { return getOperand(0); }
};

FIDDLE()
struct IRSpecializeDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRComInterfaceDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRCOMWitnessDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getWitnessTable() { return getOperand(0); }
};

/// A decoration on `IRParam`s that represent generic parameters,
/// marking the interface type that the generic parameter conforms to.
/// A generic parameter can have more than one `IRTypeConstraintDecoration`s
FIDDLE()
struct IRTypeConstraintDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getConstraintType() { return getOperand(0); }
};

bool isSimpleDecoration(IROp op);

FIDDLE()
struct IRRequireGLSLVersionDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getLanguageVersionOperand() { return cast<IRConstant>(getOperand(0)); }

    Int getLanguageVersion() { return Int(getLanguageVersionOperand()->value.intVal); }
};

FIDDLE()
struct IRSPIRVNonUniformResourceDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getSPIRVNonUniformResourceOperand() { return cast<IRConstant>(getOperand(0)); }
    IntegerLiteralValue getSPIRVNonUniformResource()
    {
        return getSPIRVNonUniformResourceOperand()->value.intVal;
    }
};

FIDDLE()
struct IRRequireSPIRVVersionDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getSPIRVVersionOperand() { return cast<IRConstant>(getOperand(0)); }
    SemanticVersion getSPIRVVersion()
    {
        return SemanticVersion::fromRaw(getSPIRVVersionOperand()->value.intVal);
    }
};

FIDDLE()
struct IRRequireCapabilityAtomDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getCapabilityAtomOperand() { return cast<IRConstant>(getOperand(0)); }
    CapabilityName getAtom() { return (CapabilityName)getCapabilityAtomOperand()->value.intVal; }
};

FIDDLE()
struct IRRequireCUDASMVersionDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getCUDASMVersionOperand() { return cast<IRConstant>(getOperand(0)); }
    SemanticVersion getCUDASMVersion()
    {
        return SemanticVersion::fromRaw(getCUDASMVersionOperand()->value.intVal);
    }
};

FIDDLE()
struct IRRequireGLSLExtensionDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getExtensionNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getExtensionName() { return getExtensionNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRRequireWGSLExtensionDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getExtensionNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getExtensionName() { return getExtensionNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRMemoryQualifierSetDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRIntegerValue getMemoryQualifierBit() { return cast<IRIntLit>(getOperand(0))->getValue(); }
};

FIDDLE()
struct IRAvailableInDownstreamIRDecoration : IRDecoration
{
    FIDDLE(leafInst())
    CodeGenTarget getTarget()
    {
        return static_cast<CodeGenTarget>(cast<IRIntLit>(getOperand(0))->getValue());
    }
};

FIDDLE()
struct IRGLSLLocationDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRIntLit* getLocation() { return cast<IRIntLit>(getOperand(0)); }
};


FIDDLE()
struct IRGLSLOffsetDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRIntLit* getOffset() { return cast<IRIntLit>(getOperand(0)); }
};

FIDDLE()
struct IRVkStructOffsetDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRIntLit* getOffset() { return cast<IRIntLit>(getOperand(0)); }
};

FIDDLE()
struct IRNVAPIMagicDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getName() { return getNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRNVAPISlotDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getRegisterNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getRegisterName() { return getRegisterNameOperand()->getStringSlice(); }

    IRStringLit* getSpaceNameOperand() { return cast<IRStringLit>(getOperand(1)); }
    UnownedStringSlice getSpaceName() { return getSpaceNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRMaxTessFactorDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRFloatLit* getMaxTessFactor() { return cast<IRFloatLit>(getOperand(0)); }
};

FIDDLE()
struct IROutputControlPointsDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getControlPointCount() { return cast<IRIntLit>(getOperand(0)); }
};

// This is used for mesh shaders too
FIDDLE()
struct IROutputTopologyDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getTopology() { return cast<IRStringLit>(getOperand(0)); }

    IRIntegerValue getTopologyType() { return cast<IRIntLit>(getOperand(1))->getValue(); }
};

FIDDLE()
struct IRPartitioningDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getPartitioning() { return cast<IRStringLit>(getOperand(0)); }
};

FIDDLE()
struct IRDomainDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getDomain() { return cast<IRStringLit>(getOperand(0)); }
};

FIDDLE()
struct IRMaxVertexCountDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getCount() { return cast<IRIntLit>(getOperand(0)); }
};

FIDDLE()
struct IRInstanceDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getCount() { return cast<IRIntLit>(getOperand(0)); }
};

struct IRGlobalParam;
FIDDLE()
struct IRNumThreadsDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getX() { return as<IRIntLit>(getOperand(0)); }
    IRIntLit* getY() { return as<IRIntLit>(getOperand(1)); }
    IRIntLit* getZ() { return as<IRIntLit>(getOperand(2)); }

    IRGlobalParam* getXSpecConst() { return as<IRGlobalParam>(getOperand(0)); }
    IRGlobalParam* getYSpecConst() { return as<IRGlobalParam>(getOperand(1)); }
    IRGlobalParam* getZSpecConst() { return as<IRGlobalParam>(getOperand(2)); }
};

FIDDLE()
struct IRFpDenormalPreserveDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRIntLit* getWidth() { return cast<IRIntLit>(getOperand(0)); }
};

FIDDLE()
struct IRFpDenormalFlushToZeroDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getWidth() { return cast<IRIntLit>(getOperand(0)); }
};

FIDDLE()
struct IRWaveSizeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getNumLanes() { return cast<IRIntLit>(getOperand(0)); }
};

FIDDLE()
struct IREntryPointDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getProfileInst() { return cast<IRIntLit>(getOperand(0)); }
    Profile getProfile() { return Profile(Profile::RawVal(getIntVal(getProfileInst()))); }

    IRStringLit* getName() { return cast<IRStringLit>(getOperand(1)); }
    IRStringLit* getModuleName() { return cast<IRStringLit>(getOperand(2)); }
    void setName(IRStringLit* name) { setOperand(1, name); }
};

FIDDLE()
struct IRCudaKernelForwardDerivativeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getForwardDerivativeFunc() { return getOperand(0); }
};

FIDDLE()
struct IRCudaKernelBackwardDerivativeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getBackwardDerivativeFunc() { return getOperand(0); }
};

FIDDLE()
struct IRGeometryInputPrimitiveTypeDecoration : IRDecoration
{
    FIDDLE(baseInst())
};

/// This is a bit of a hack. The problem is that when GLSL legalization takes place
/// the parameters from the entry point are globalized *and* potentially split
/// So even if we did copy a suitable decoration onto the globalized parameters,
/// it would potentially output multiple times without extra logic.
/// Using this decoration we can copy the StreamOut type to the entry point, and then
/// emit as part of entry point attribute emitting.
FIDDLE()
struct IRStreamOutputTypeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRHLSLStreamOutputType* getStreamType() { return cast<IRHLSLStreamOutputType>(getOperand(0)); }
};

/// A decoration that marks a value as having linkage.
/// A value with linkage is either exported from its module,
/// or will have a definition imported from another module.
/// In either case, it requires a mangled name to use when
/// matching imports and exports.
FIDDLE()
struct IRLinkageDecoration : IRDecoration
{
    FIDDLE(baseInst())

    IRStringLit* getMangledNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getMangledName() { return getMangledNameOperand()->getStringSlice(); }
};

// Mark a global variable as a target buitlin variable.
FIDDLE()
struct IRTargetBuiltinVarDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getBuiltinVarOperand() { return cast<IRIntLit>(getOperand(0)); }
    IRTargetBuiltinVarName getBuiltinVarName()
    {
        return IRTargetBuiltinVarName(getBuiltinVarOperand()->getValue());
    }
};

FIDDLE()
struct IRUserExternDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRImportDecoration : IRLinkageDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRExportDecoration : IRLinkageDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRExternCppDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getName() { return getNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRExternCDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRDllImportDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getLibraryNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getLibraryName() { return getLibraryNameOperand()->getStringSlice(); }

    IRStringLit* getFunctionNameOperand() { return cast<IRStringLit>(getOperand(1)); }
    UnownedStringSlice getFunctionName() { return getFunctionNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRDllExportDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getFunctionNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getFunctionName() { return getFunctionNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRTorchEntryPointDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getFunctionNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getFunctionName() { return getFunctionNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRAutoPyBindCudaDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getFunctionNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getFunctionName() { return getFunctionNameOperand()->getStringSlice(); }
};

FIDDLE()
struct IRPyExportDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getExportNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getExportName() { return getExportNameOperand()->getStringSlice(); }
};


FIDDLE()
struct IRKnownBuiltinDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getNameOperand() { return cast<IRIntLit>(getOperand(0)); }
    KnownBuiltinDeclName getName() { return KnownBuiltinDeclName(getIntVal(getNameOperand())); }
};

FIDDLE()
struct IREntryPointParamDecoration : IRDecoration
{
    FIDDLE(leafInst())

    /// Get the entry point that this parameter orignates from.
    IRFunc* getEntryPoint() { return cast<IRFunc>(getOperand(0)); }
};

FIDDLE()
struct IRFormatDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRConstant* getFormatOperand() { return cast<IRConstant>(getOperand(0)); }

    ImageFormat getFormat() { return ImageFormat(getFormatOperand()->value.intVal); }
};

FIDDLE()
struct IRSizeAndAlignmentDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRTypeLayoutRuleName getLayoutName()
    {
        return IRTypeLayoutRuleName(cast<IRIntLit>(getOperand(0))->getValue());
    }

    IRIntLit* getSizeOperand() { return cast<IRIntLit>(getOperand(1)); }
    IRIntLit* getAlignmentOperand() { return cast<IRIntLit>(getOperand(2)); }
    IRIntegerValue getSize() { return getSizeOperand()->getValue(); }
    IRIntegerValue getAlignment() { return getAlignmentOperand()->getValue(); }
};

FIDDLE()
struct IROffsetDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRTypeLayoutRuleName getLayoutName()
    {
        return IRTypeLayoutRuleName(cast<IRIntLit>(getOperand(0))->getValue());
    }

    IRIntLit* getOffsetOperand() { return cast<IRIntLit>(getOperand(1)); }
    IRIntegerValue getOffset() { return getOffsetOperand()->getValue(); }
};

FIDDLE()
struct IRBuiltinDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRSequentialIDDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getSequentialIDOperand() { return cast<IRIntLit>(getOperand(0)); }
    IRIntegerValue getSequentialID() { return getSequentialIDOperand()->getValue(); }
};

FIDDLE()
struct IRResultWitnessDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getWitness() { return getOperand(0); }
};

FIDDLE()
struct IRDynamicDispatchWitnessDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRAutoDiffOriginalValueDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRInst* getOriginalValue() { return getOperand(0); }
};

FIDDLE()
struct IRForwardDifferentiableDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRForwardDerivativeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getForwardDerivativeFunc() { return getOperand(0); }
};

FIDDLE()
struct IRPrimalSubstituteDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getPrimalSubstituteFunc() { return getOperand(0); }
};

FIDDLE()
struct IRBackwardDerivativeIntermediateTypeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getBackwardDerivativeIntermediateType() { return getOperand(0); }
};

FIDDLE()
struct IRBackwardDerivativePrimalDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getBackwardDerivativePrimalFunc() { return getOperand(0); }
};

// Used to associate the restore context var to use in a call to splitted backward propgate
// function.
FIDDLE()
struct IRBackwardDerivativePrimalContextDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRUse primalContextVar;
    IRInst* getBackwardDerivativePrimalContextVar() { return getOperand(0); }
};

FIDDLE()
struct IRBackwardDerivativePrimalReturnDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getBackwardDerivativePrimalReturnValue() { return getOperand(0); }
};

FIDDLE()
struct IRBackwardDerivativePropagateDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getBackwardDerivativePropagateFunc() { return getOperand(0); }
};

FIDDLE()
struct IRBackwardDerivativeDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getBackwardDerivativeFunc() { return getOperand(0); }
};

FIDDLE()
struct IRCheckpointHintDecoration : public IRDecoration
{
    FIDDLE(baseInst())
};

FIDDLE()
struct IRPreferRecomputeDecoration : IRCheckpointHintDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRPreferCheckpointDecoration : IRCheckpointHintDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRCheckpointIntermediateDecoration : IRCheckpointHintDecoration
{
    FIDDLE(leafInst())

    IRInst* getSourceFunction() { return getOperand(0); }
};

FIDDLE()
struct IRLoopCounterDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRLoopCounterUpdateDecoration : IRDecoration
{
    FIDDLE(leafInst())
};


FIDDLE()
struct IRLoopExitPrimalValueDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRUse target;
    IRUse exitVal;
    IRInst* getTargetInst() { return getOperand(0); }
    IRInst* getLoopExitValInst() { return getOperand(1); }
};

FIDDLE()
struct IRAutodiffInstDecoration : IRDecoration
{
    FIDDLE(baseInst())
};

FIDDLE()
struct IRDifferentialInstDecoration : IRAutodiffInstDecoration
{
    FIDDLE(leafInst())

    IRUse primalType;

    IRType* getPrimalType() { return (IRType*)(getOperand(0)); }
    IRInst* getPrimalInst() { return getOperand(1); }
    IRInst* getWitness() { return getOperand(2); }
};

FIDDLE()
struct IRPrimalInstDecoration : IRAutodiffInstDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMixedDifferentialInstDecoration : IRAutodiffInstDecoration
{
    FIDDLE(leafInst())

    IRUse pairType;

    IRType* getPairType() { return (IRType*)(getOperand(0)); }
};

FIDDLE()
struct IRRecomputeBlockDecoration : IRAutodiffInstDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRPrimalValueStructKeyDecoration : IRDecoration
{
    FIDDLE(leafInst())


    IRStructKey* getStructKey() { return as<IRStructKey>(getOperand(0)); }
};

FIDDLE()
struct IRPrimalElementTypeDecoration : IRDecoration
{
    FIDDLE(leafInst())


    IRInst* getPrimalElementType() { return getOperand(0); }
};

FIDDLE()
struct IRIntermediateContextFieldDifferentialTypeDecoration : IRDecoration
{
    FIDDLE(leafInst())


    IRInst* getDifferentialWitness() { return getOperand(0); }
};

FIDDLE()
struct IRBackwardDifferentiableDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRUserDefinedBackwardDerivativeDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRInst* getBackwardDerivativeFunc() { return getOperand(0); }
};

FIDDLE()
struct IRTreatAsDifferentiableDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

// Mark a call as explicitly calling a differentiable function.
FIDDLE()
struct IRDifferentiableCallDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

// Mark a type as being eligible for trimming if necessary. If
// any fields don't have any effective loads from them, they can be
// removed.
//
FIDDLE()
struct IROptimizableTypeDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

// Informs the DCE pass to ignore side-effects on this call for
// the purposes of dead code elimination, even if the call does have
// side-effects.
//
FIDDLE()
struct IRIgnoreSideEffectsDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

// Treat a call to a non-differentiable function as a differentiable call.
FIDDLE()
struct IRTreatCallAsDifferentiableDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRDerivativeMemberDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getDerivativeMemberStructKey() { return getOperand(0); }
};

// An instruction that replaces the function symbol
// with it's derivative function.
FIDDLE()
struct IRForwardDifferentiate : IRInst
{
    FIDDLE(leafInst())
    // The base function for the call.
    IRUse base;
    IRInst* getBaseFn() { return getOperand(0); }
};

// An instruction that replaces the function symbol
// with its backward derivative primal function.
// A backward derivative primal function is the first pass
// of backward derivative computation. It performs the primal
// computations and returns the intermediates that will be used
// by the actual backward derivative function.
FIDDLE()
struct IRBackwardDifferentiatePrimal : IRInst
{
    FIDDLE(leafInst())
    // The base function for the call.
    IRUse base;
    IRInst* getBaseFn() { return getOperand(0); }
};

// An instruction that replaces the function symbol with its backward derivative propagate function.
// A backward derivative propagate function is the second pass of backward derivative computation.
// It uses the intermediates computed in the bacward derivative primal function to perform the
// actual backward derivative propagation.
FIDDLE()
struct IRBackwardDifferentiatePropagate : IRInst
{
    FIDDLE(leafInst())
    // The base function for the call.
    IRUse base;
    IRInst* getBaseFn() { return getOperand(0); }
};

// An instruction that replaces the function symbol with its backward derivative function.
// A backward derivative function is a concept that combines both passes of backward derivative
// computation. This inst should only be produced by lower-to-ir, and will be replaced with calls to
// the primal function followed by the propagate function in the auto-diff pass.
FIDDLE()
struct IRBackwardDifferentiate : IRInst
{
    FIDDLE(leafInst())
    // The base function for the call.
    IRUse base;
    IRInst* getBaseFn() { return getOperand(0); }
};

FIDDLE()
struct IRIsDifferentialNull : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBase() { return getOperand(0); }
};

// Retrieves the primal substitution function for the given function.
FIDDLE()
struct IRPrimalSubstitute : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBaseFn() { return getOperand(0); }
};

FIDDLE()
struct IRDifferentiableTypeAnnotation : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBaseType() { return getOperand(0); }
    IRInst* getWitness() { return getOperand(1); }
};

FIDDLE()
struct IRDispatchKernel : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBaseFn() { return getOperand(0); }
    IRInst* getThreadGroupSize() { return getOperand(1); }
    IRInst* getDispatchSize() { return getOperand(2); }
    UInt getArgCount() { return getOperandCount() - 3; }
    IRInst* getArg(UInt i) { return getOperand(3 + i); }
    IROperandList<IRInst> getArgsList()
    {
        return IROperandList<IRInst>(getOperands() + 3, getOperands() + getOperandCount());
    }
};

FIDDLE()
struct IRTorchTensorGetView : IRInst
{
    FIDDLE(leafInst())
};

// Dictionary item mapping a type with a corresponding
// IDifferentiable witness table
//
FIDDLE()
struct IRDifferentiableTypeDictionaryItem : IRInst
{
    FIDDLE(leafInst())

    IRInst* getConcreteType() { return getOperand(0); }
    IRInst* getWitness() { return getOperand(1); }
};

FIDDLE()
struct IRDifferentiableTypeDictionaryDecoration : IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRFloatingPointModeOverrideDecoration : IRDecoration
{
    FIDDLE(leafInst())

    FloatingPointMode getFloatingPointMode()
    {
        return (FloatingPointMode)cast<IRIntLit>(getOperand(0))->getValue();
    }
};

// An instruction that specializes another IR value
// (representing a generic) to a particular set of generic arguments
// (instructions representing types, witness tables, etc.)
FIDDLE()
struct IRSpecialize : IRInst
{
    FIDDLE(leafInst())
    // The "base" for the call is the generic to be specialized
    IRUse base;
    IRInst* getBase() { return getOperand(0); }

    // after the generic value come the arguments
    UInt getArgCount() { return getOperandCount() - 1; }
    IRInst* getArg(UInt index) { return getOperand(index + 1); }
    IRUse* getArgOperand(Index i) { return getOperands() + 1 + i; }
};

// An instruction that looks up the implementation
// of an interface operation identified by `requirementDeclRef`
// in the witness table `witnessTable` which should
// hold the conformance information for a specific type.
FIDDLE()
struct IRLookupWitnessMethod : IRInst
{
    FIDDLE(leafInst())
    IRUse witnessTable;
    IRUse requirementKey;

    IRInst* getWitnessTable() { return witnessTable.get(); }
    IRInst* getRequirementKey() { return requirementKey.get(); }
};

// Returns the sequential ID of an RTTI object.
FIDDLE()
struct IRGetSequentialID : IRInst
{
    FIDDLE(leafInst())

    IRInst* getRTTIOperand() { return getOperand(0); }
};

/// Allocates space from local stack.
///
FIDDLE()
struct IRAlloca : IRInst
{
    FIDDLE(leafInst())

    IRInst* getAllocSize() { return getOperand(0); }
};

/// A non-hoistable inst used to "pin" a global value inside a function body so any insts dependent
/// on `value` can be emitted as local insts instead of global insts, as required by targets (e.g.
/// spirv) that doesn't allow the dependent computation in the global scope.
///
FIDDLE()
struct IRGlobalValueRef : IRInst
{
    FIDDLE(leafInst())

    IRInst* getValue() { return getOperand(0); }
};

/// Packs a value into an `AnyValue`.
/// Return type is `IRAnyValueType`.
FIDDLE()
struct IRPackAnyValue : IRInst
{
    FIDDLE(leafInst())
    IRInst* getValue() { return getOperand(0); }
};

/// Unpacks a `AnyValue` value into a concrete type.
/// Operand must have `IRAnyValueType`.
FIDDLE()
struct IRUnpackAnyValue : IRInst
{
    FIDDLE(leafInst())
    IRInst* getValue() { return getOperand(0); }
};

FIDDLE()
struct IRBitFieldAccessorDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRStructKey* getBackingMemberKey() { return cast<IRStructKey>(getOperand(0)); }
    IRIntegerValue getFieldWidth() { return as<IRIntLit>(getOperand(1))->getValue(); }
    IRIntegerValue getFieldOffset() { return as<IRIntLit>(getOperand(2))->getValue(); }
};

// Layout decorations

/// A decoration that marks a field key as having been associated
/// with a particular simple semantic (e.g., `COLOR` or `SV_Position`,
/// but not a `register` semantic).
///
/// This is currently needed so that we can round-trip HLSL `struct`
/// types that get used for varying input/output. This is an unfortunate
/// case where some amount of "layout" information can't just come
/// in via the `TypeLayout` part of things.
///
FIDDLE()
struct IRSemanticDecoration : public IRDecoration
{
    FIDDLE(leafInst())

    IRStringLit* getSemanticNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getSemanticName() { return getSemanticNameOperand()->getStringSlice(); }

    IRIntLit* getSemanticIndexOperand() { return cast<IRIntLit>(getOperand(1)); }
    int getSemanticIndex() { return int(getIntVal(getSemanticIndexOperand())); }
};

FIDDLE()
struct IRConstructorDecoration : IRDecoration
{
    FIDDLE(leafInst())

    bool getSynthesizedStatus() { return cast<IRBoolLit>(getOperand(0))->getValue(); }
};

FIDDLE()
struct IRPackOffsetDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRIntLit* getRegisterOffset() { return cast<IRIntLit>(getOperand(0)); }
    IRIntLit* getComponentOffset() { return cast<IRIntLit>(getOperand(1)); }
};

FIDDLE()
struct IRUserTypeNameDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRStringLit* getUserTypeName() { return cast<IRStringLit>(getOperand(0)); }
};

FIDDLE()
struct IRCounterBufferDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRInst* getCounterBuffer() { return getOperand(0); }
};

FIDDLE()
struct IRStageAccessDecoration : public IRDecoration
{
    FIDDLE(baseInst())
    Int getStageCount() { return (Int)getOperandCount(); }
    IRStringLit* getStageOperand(Int index) { return cast<IRStringLit>(getOperand(index)); }
    UnownedStringSlice getStageName(Int index) { return getStageOperand(index)->getStringSlice(); }
};

FIDDLE()
struct IRStageReadAccessDecoration : public IRStageAccessDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRStageWriteAccessDecoration : public IRStageAccessDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRRayPayloadDecoration : public IRDecoration
{
    FIDDLE(leafInst())
};

// Mesh shader decorations

FIDDLE()
struct IRMeshOutputDecoration : public IRDecoration
{
    FIDDLE(baseInst())
    IRIntLit* getMaxSize() { return cast<IRIntLit>(getOperand(0)); }
};

FIDDLE()
struct IRVerticesDecoration : public IRMeshOutputDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRIndicesDecoration : public IRMeshOutputDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRPrimitivesDecoration : public IRMeshOutputDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRGLSLPrimitivesRateDecoration : public IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRGLPositionOutputDecoration : public IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRGLPositionInputDecoration : public IRDecoration
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMeshOutputRef : public IRInst
{
    FIDDLE(leafInst())
    IRInst* getBase() { return getOperand(0); }
    IRInst* getIndex() { return getOperand(1); }
    IRInst* getOutputType() { return cast<IRPtrTypeBase>(getFullType())->getValueType(); }
};

FIDDLE()
struct IRMeshOutputSet : public IRInst
{
    FIDDLE(leafInst())
    IRInst* getBase() { return getOperand(0); }
    IRInst* getIndex() { return getOperand(1); }
    IRInst* getElementValue() { return getOperand(2); }
};

FIDDLE()
struct IRMetalSetVertex : public IRInst
{
    FIDDLE(leafInst())
    IRInst* getIndex() { return getOperand(0); }
    IRInst* getElementValue() { return getOperand(1); }
};

FIDDLE()
struct IRMetalSetPrimitive : public IRInst
{
    FIDDLE(leafInst())
    IRInst* getIndex() { return getOperand(0); }
    IRInst* getElementValue() { return getOperand(1); }
};

FIDDLE()
struct IRMetalSetIndices : public IRInst
{
    FIDDLE(leafInst())
    IRInst* getIndex() { return getOperand(0); }
    IRInst* getElementValue() { return getOperand(1); }
};

/// An attribute that can be attached to another instruction as an operand.
///
/// Attributes serve a similar role to decorations, in that both are ways
/// to attach additional information to an instruction, where the operand
/// of the attribute/decoration identifies the purpose of the additional
/// information.
///
/// The key difference between decorations and attributes is that decorations
/// are stored as children of an instruction (in terms of the ownership
/// hierarchy), while attributes are referenced as operands.
///
/// The key benefit of having attributes be operands is that they must
/// be present at the time an instruction is created, which means that
/// they can affect the conceptual value/identity of an instruction
/// in cases where we deduplicate/hash instructions by value.
///
FIDDLE()
struct IRAttr : public IRInst
{
    FIDDLE(baseInst())
};

/// An attribute that specifies layout information for a single resource kind.
FIDDLE()
struct IRLayoutResourceInfoAttr : public IRAttr
{
    FIDDLE(baseInst())

    IRIntLit* getResourceKindInst() { return cast<IRIntLit>(getOperand(0)); }
    LayoutResourceKind getResourceKind()
    {
        return LayoutResourceKind(getIntVal(getResourceKindInst()));
    }
};

/// An attribute that specifies offset information for a single resource kind.
///
/// This operation can appear as `varOffset(kind, offset)` or
/// `varOffset(kind, offset, space)`. The latter form is only
/// used when `space` is non-zero.
///
FIDDLE()
struct IRVarOffsetAttr : public IRLayoutResourceInfoAttr
{
    FIDDLE(leafInst())

    IRIntLit* getOffsetInst() { return cast<IRIntLit>(getOperand(1)); }
    UInt getOffset() { return UInt(getIntVal(getOffsetInst())); }

    IRIntLit* getSpaceInst()
    {
        if (getOperandCount() > 2)
            return cast<IRIntLit>(getOperand(2));
        return nullptr;
    }

    UInt getSpace()
    {
        if (auto spaceInst = getSpaceInst())
            return UInt(getIntVal(spaceInst));
        return 0;
    }
};

/// An attribute that specifies the error type a function is throwing
FIDDLE()
struct IRFuncThrowTypeAttr : IRAttr
{
    FIDDLE(leafInst())

    IRType* getErrorType() { return (IRType*)getOperand(0); }
};

FIDDLE()
struct IRNoDiffAttr : IRAttr
{
    FIDDLE(leafInst())
};

/// An attribute that specifies size information for a single resource kind.
FIDDLE()
struct IRTypeSizeAttr : public IRLayoutResourceInfoAttr
{
    FIDDLE(leafInst())

    IRIntLit* getSizeInst() { return cast<IRIntLit>(getOperand(1)); }
    LayoutSize getSize()
    {
        return LayoutSize::fromRaw(LayoutSize::RawValue(getIntVal(getSizeInst())));
    }
    size_t getFiniteSize() { return getSize().getFiniteValue(); }
};

// Layout

/// Base type for instructions that represent layout information.
///
/// Layout instructions are effectively just meta-data constants.
///
FIDDLE()
struct IRLayout : IRInst
{
    FIDDLE(baseInst())
};

struct IRVarLayout;

/// An attribute to specify that a layout has another layout attached for "pending" data.
///
/// "Pending" data refers to the parts of a type or variable that
/// couldn't be laid out until the concrete types for existential
/// type slots were filled in. The layout of pending data may not
/// be contiguous with the layout of the original type/variable.
///
FIDDLE()
struct IRPendingLayoutAttr : IRAttr
{
    FIDDLE(leafInst())

    IRLayout* getLayout() { return cast<IRLayout>(getOperand(0)); }
};

/// Layout information for a type.
///
/// The most important thing this instruction provides is the
/// resource usage (aka "size") of the type for each of the
/// resource kinds it consumes.
///
/// Subtypes of `IRTypeLayout` will include additional type-specific
/// operands or attributes. For example, a type layout for a
/// `struct` type will include offset information for its fields.
///
FIDDLE()
struct IRTypeLayout : IRLayout
{
    FIDDLE(baseInst())

    /// Find the attribute that stores offset information for `kind`.
    ///
    /// Returns null if no attribute is found, indicating that this
    /// type does not consume any resources of `kind`.
    ///
    IRTypeSizeAttr* findSizeAttr(LayoutResourceKind kind);

    /// Get all the attributes representing size information.
    IROperandList<IRTypeSizeAttr> getSizeAttrs();

    /// Unwrap any layers of array-ness and return the outer-most non-array type.
    IRTypeLayout* unwrapArray();

    /// Get the layout for pending data, if present.
    IRTypeLayout* getPendingDataTypeLayout();

    /// A builder for constructing `IRTypeLayout`s
    struct Builder
    {
        /// Begin building.
        ///
        /// The `irBuilder` will be used to construct the
        /// type layout and any additional instructions required.
        ///
        Builder(IRBuilder* irBuilder);

        /// Add `size` units of resource `kind` to the resource usage of this type.
        void addResourceUsage(LayoutResourceKind kind, LayoutSize size);

        /// Add the resource usage specified by `sizeAttr`.
        void addResourceUsage(IRTypeSizeAttr* sizeAttr);

        /// Add all resource usage from `typeLayout`.
        void addResourceUsageFrom(IRTypeLayout* typeLayout);

        /// Set the (optional) layout for pending data.
        void setPendingTypeLayout(IRTypeLayout* typeLayout) { m_pendingTypeLayout = typeLayout; }

        /// Build a type layout according to the information specified so far.
        IRTypeLayout* build();

    protected:
        // The following services are provided so that
        // subtypes of `IRTypeLayout` can provide their
        // own `Builder` subtypes that construct appropriate
        // layouts.

        /// Override to customize the opcode of the generated layout.
        virtual IROp getOp() { return kIROp_TypeLayoutBase; }

        /// Override to add additional operands to the generated layout.
        virtual void addOperandsImpl(List<IRInst*>&) {}

        /// Override to add additional attributes to the generated layout.
        virtual void addAttrsImpl(List<IRInst*>&) {}

        /// Use to access the underlying IR builder.
        IRBuilder* getIRBuilder() { return m_irBuilder; };

    private:
        void addOperands(List<IRInst*>&);
        void addAttrs(List<IRInst*>& ioOperands);

        IRBuilder* m_irBuilder = nullptr;
        IRTypeLayout* m_pendingTypeLayout = nullptr;

        struct ResInfo
        {
            LayoutResourceKind kind = LayoutResourceKind::None;
            LayoutSize size = 0;
        };
        ResInfo m_resInfos[SLANG_PARAMETER_CATEGORY_COUNT];
    };
};

/// Type layout for parameter groups (constant buffers and parameter blocks)
FIDDLE()
struct IRParameterGroupTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())
private:
    typedef IRTypeLayout Super;

public:
    IRVarLayout* getContainerVarLayout() { return cast<IRVarLayout>(getOperand(0)); }

    IRVarLayout* getElementVarLayout() { return cast<IRVarLayout>(getOperand(1)); }

    // TODO: There shouldn't be a need for the IR to store an "offset" element type layout,
    // but there are just enough places that currently use that information so that removing
    // it would require some careful refactoring.
    //
    IRTypeLayout* getOffsetElementTypeLayout() { return cast<IRTypeLayout>(getOperand(2)); }

    /// Specialized builder for parameter group type layouts.
    struct Builder : Super::Builder
    {
    public:
        Builder(IRBuilder* irBuilder)
            : Super::Builder(irBuilder)
        {
        }

        void setContainerVarLayout(IRVarLayout* varLayout) { m_containerVarLayout = varLayout; }

        void setElementVarLayout(IRVarLayout* varLayout) { m_elementVarLayout = varLayout; }

        void setOffsetElementTypeLayout(IRTypeLayout* typeLayout)
        {
            m_offsetElementTypeLayout = typeLayout;
        }

        IRParameterGroupTypeLayout* build();

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_ParameterGroupTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRVarLayout* m_containerVarLayout;
        IRVarLayout* m_elementVarLayout;
        IRTypeLayout* m_offsetElementTypeLayout;
    };
};

/// Specialized layout information for array types
FIDDLE()
struct IRArrayTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())
    typedef IRTypeLayout Super;


    IRTypeLayout* getElementTypeLayout() { return cast<IRTypeLayout>(getOperand(0)); }

    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder, IRTypeLayout* elementTypeLayout)
            : Super::Builder(irBuilder), m_elementTypeLayout(elementTypeLayout)
        {
        }

        IRArrayTypeLayout* build() { return cast<IRArrayTypeLayout>(Super::Builder::build()); }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_ArrayTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRTypeLayout* m_elementTypeLayout;
    };
};

/// Specialized layout information for structured buffer types
FIDDLE()
struct IRStructuredBufferTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())
    typedef IRTypeLayout Super;


    IRTypeLayout* getElementTypeLayout() { return cast<IRTypeLayout>(getOperand(0)); }

    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder, IRTypeLayout* elementTypeLayout)
            : Super::Builder(irBuilder), m_elementTypeLayout(elementTypeLayout)
        {
        }

        IRStructuredBufferTypeLayout* build()
        {
            return cast<IRStructuredBufferTypeLayout>(Super::Builder::build());
        }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_StructuredBufferTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) override;

        IRTypeLayout* m_elementTypeLayout;
    };
};

/* TODO(JS):

It would arguably be "more correct" if the IRPointerTypeLayout, contained a refence to the
value/target type layout. Ie...

```
IRTypeLayout* m_valueTypeLayout;
```

Unfortunately that doesn't work because it leads to an infinite loop if the target contains a Ptr to
the containing struct.

This isn't so simple to fix (as has been done with similar problems elsewhere), because Layout
also hoists/deduped layouts.

As it stands the "attributes" describing the layout fields are held as operands and as such are part
of the hash that is used for deduping. That makes sense (if the fields change depending on where/how
a struct type is used), but creates a problem because we can't lookup the type until it is
"complete" (ie has all the fields) and we can't have all the fields if one is a pointer that causes
infinite recursion in lookup.

The work around for now is to observe that layout of a Ptr doesn't depend on what is being pointed
to and as such we don't store the this in the pointer.
*/
FIDDLE()
struct IRPointerTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())
    typedef IRTypeLayout Super;


    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder)
            : Super::Builder(irBuilder)
        {
        }

        IRPointerTypeLayout* build() { return cast<IRPointerTypeLayout>(Super::Builder::build()); }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_PointerTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;
    };
};

/// Specialized layout information for stream-output types
FIDDLE()
struct IRStreamOutputTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())
    typedef IRTypeLayout Super;


    IRTypeLayout* getElementTypeLayout() { return cast<IRTypeLayout>(getOperand(0)); }

    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder, IRTypeLayout* elementTypeLayout)
            : Super::Builder(irBuilder), m_elementTypeLayout(elementTypeLayout)
        {
        }

        IRArrayTypeLayout* build() { return cast<IRArrayTypeLayout>(Super::Builder::build()); }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_StreamOutputTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRTypeLayout* m_elementTypeLayout;
    };
};

/// Specialized layout information for matrix types
FIDDLE()
struct IRMatrixTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())
    typedef IRTypeLayout Super;


    MatrixLayoutMode getMode()
    {
        return MatrixLayoutMode(getIntVal(cast<IRIntLit>(getOperand(0))));
    }

    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder, MatrixLayoutMode mode);

        IRMatrixTypeLayout* build() { return cast<IRMatrixTypeLayout>(Super::Builder::build()); }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_MatrixTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRInst* m_modeInst = nullptr;
    };
};

/// Attribute that specifies the layout for one field of a structure type.
FIDDLE()
struct IRStructFieldLayoutAttr : IRAttr
{
    FIDDLE(leafInst())

    IRInst* getFieldKey() { return getOperand(0); }

    IRVarLayout* getLayout() { return cast<IRVarLayout>(getOperand(1)); }
};

/// Specialized layout information for structure types.
FIDDLE()
struct IRStructTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())

    typedef IRTypeLayout Super;

    /// Get all of the attributes that represent field layouts.
    IROperandList<IRStructFieldLayoutAttr> getFieldLayoutAttrs()
    {
        return findAttrs<IRStructFieldLayoutAttr>();
    }

    /// Get the number of fields for which layout information is stored.
    UInt getFieldCount() { return getFieldLayoutAttrs().getCount(); }

    /// Get the layout information for a field by `index`
    IRVarLayout* getFieldLayout(UInt index) { return getFieldLayoutAttrs()[index]->getLayout(); }

    /// Specialized builder for structure type layouts.
    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder)
            : Super::Builder(irBuilder)
        {
        }

        void addField(IRInst* key, IRVarLayout* layout)
        {
            FieldInfo info;
            info.key = key;
            info.layout = layout;
            m_fields.add(info);
        }

        IRStructTypeLayout* build() { return cast<IRStructTypeLayout>(Super::Builder::build()); }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_StructTypeLayout; }
        void addAttrsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        struct FieldInfo
        {
            IRInst* key;
            IRVarLayout* layout;
        };

        List<FieldInfo> m_fields;
    };
};

/// Attribute that specifies the layout for one field of a structure type.
FIDDLE()
struct IRTupleFieldLayoutAttr : IRAttr
{
    FIDDLE(leafInst())

    IRTypeLayout* getLayout() { return cast<IRTypeLayout>(getOperand(1)); }
};

/// Specialized layout information for tuple types.
FIDDLE()
struct IRTupleTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())

    typedef IRTypeLayout Super;

    /// Get all of the attributes that represent field layouts.
    IROperandList<IRTupleFieldLayoutAttr> getFieldLayoutAttrs()
    {
        return findAttrs<IRTupleFieldLayoutAttr>();
    }

    /// Get the number of fields for which layout information is stored.
    UInt getFieldCount() { return getFieldLayoutAttrs().getCount(); }

    /// Get the layout information for a field by `index`
    IRTypeLayout* getFieldLayout(UInt index) { return getFieldLayoutAttrs()[index]->getLayout(); }

    /// Specialized builder for tuple type layouts.
    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder)
            : Super::Builder(irBuilder)
        {
        }

        void addField(IRTypeLayout* layout)
        {
            FieldInfo info;
            info.layout = layout;
            m_fields.add(info);
        }

        IRTupleTypeLayout* build() { return cast<IRTupleTypeLayout>(Super::Builder::build()); }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_TupleTypeLayout; }
        void addAttrsImpl(List<IRInst*>& ioOperands) override;

        struct FieldInfo
        {
            IRTypeLayout* layout;
        };

        List<FieldInfo> m_fields;
    };
};

/// Attribute that represents the layout for one case of a union type
FIDDLE()
struct IRCaseTypeLayoutAttr : IRAttr
{
    FIDDLE(leafInst())

    IRTypeLayout* getTypeLayout() { return cast<IRTypeLayout>(getOperand(0)); }
};

/// Type layout for an existential/interface type.
FIDDLE()
struct IRExistentialTypeLayout : IRTypeLayout
{
    FIDDLE(leafInst())
    typedef IRTypeLayout Super;


    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder)
            : Super::Builder(irBuilder)
        {
        }

        IRExistentialTypeLayout* build()
        {
            return cast<IRExistentialTypeLayout>(Super::Builder::build());
        }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_ExistentialTypeLayout; }
    };
};


/// Layout information for an entry point
FIDDLE()
struct IREntryPointLayout : IRLayout
{
    FIDDLE(leafInst())

    /// Get the layout information for the entry point parameters.
    ///
    /// The parameters layout will either be a structure type layout
    /// with one field per parameter, or a parameter group type
    /// layout wrapping such a structure, if the entry point parameters
    /// needed to be allocated into a constant buffer.
    ///
    IRVarLayout* getParamsLayout() { return cast<IRVarLayout>(getOperand(0)); }

    /// Get the layout information for the entry point result.
    ///
    /// This represents the return value of the entry point.
    /// Note that it does *not* represent all of the entry
    /// point outputs, because the parameter list may also
    /// contain `out` or `inout` parameters.
    ///
    IRVarLayout* getResultLayout() { return cast<IRVarLayout>(getOperand(1)); }
};

/// Given an entry-point layout, extract the layout for the parameters struct.
IRStructTypeLayout* getScopeStructLayout(IREntryPointLayout* scopeLayout);

/// Attribute that associates a variable layout with a known stage.
FIDDLE()
struct IRStageAttr : IRAttr
{
    FIDDLE(leafInst())

    IRIntLit* getStageOperand() { return cast<IRIntLit>(getOperand(0)); }
    Stage getStage() { return Stage(getIntVal(getStageOperand())); }
};

/// Base type for attributes that associate a variable layout with a semantic name and index.
FIDDLE()
struct IRSemanticAttr : IRAttr
{
    FIDDLE(baseInst())

    IRStringLit* getNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getName() { return getNameOperand()->getStringSlice(); }

    IRIntLit* getIndexOperand() { return cast<IRIntLit>(getOperand(1)); }
    UInt getIndex() { return UInt(getIntVal(getIndexOperand())); }
};

/// Attribute that associates a variable with a system-value semantic name and index
FIDDLE()
struct IRSystemValueSemanticAttr : IRSemanticAttr
{
    FIDDLE(leafInst())
};

/// Attribute that associates a variable with a user-defined semantic name and index
FIDDLE()
struct IRUserSemanticAttr : IRSemanticAttr
{
    FIDDLE(leafInst())
};

/// Layout infromation for a single parameter/field
FIDDLE()
struct IRVarLayout : IRLayout
{
    FIDDLE(leafInst())

    /// Get the type layout information for this variable
    IRTypeLayout* getTypeLayout() { return cast<IRTypeLayout>(getOperand(0)); }

    /// Get all the attributes representing resource-kind-specific offsets
    IROperandList<IRVarOffsetAttr> getOffsetAttrs();

    /// Find the offset information (if present) for the given resource `kind`
    IRVarOffsetAttr* findOffsetAttr(LayoutResourceKind kind);

    /// Does this variable use any resources of the given `kind`?
    bool usesResourceKind(LayoutResourceKind kind);
    /// Returns true if there is use of one or more of the kinds
    bool usesResourceFromKinds(LayoutResourceKindFlags kindFlags);

    /// Get the fixed/known stage that this variable is associated with.
    ///
    /// This will be a specific stage for entry-point parameters, but
    /// will be `Stage::Unknown` for any parameter that is not bound
    /// solely to one entry point.
    ///
    Stage getStage();

    /// Find the system-value semantic attribute for this variable, if any.
    IRSystemValueSemanticAttr* findSystemValueSemanticAttr();

    /// Get the (optional) layout for any "pending" data assocaited with this variable.
    IRVarLayout* getPendingVarLayout();

    /// Builder for construction `IRVarLayout`s in a stateful fashion
    struct Builder
    {
        /// Begin building a variable layout with the given `typeLayout`
        ///
        /// The result layout and any instructions needed along the way
        /// will be allocated with `irBuilder`.
        ///
        Builder(IRBuilder* irBuilder, IRTypeLayout* typeLayout);

        /// Represents resource-kind-specific offset information
        struct ResInfo
        {
            LayoutResourceKind kind = LayoutResourceKind::None;
            UInt offset = 0;
            UInt space = 0;
        };

        /// Has any resource usage/offset been registered for the given resource `kind`?
        bool usesResourceKind(LayoutResourceKind kind);

        /// Either fetch or add a `ResInfo` record for `kind` and return it
        ResInfo* findOrAddResourceInfo(LayoutResourceKind kind);

        /// Set the (optional) variable layout for pending data.
        void setPendingVarLayout(IRVarLayout* varLayout) { m_pendingVarLayout = varLayout; }

        /// Set the (optional) system-valeu semantic for this variable.
        void setSystemValueSemantic(String const& name, UInt index);

        /// Set the (optional) user-defined semantic for this variable.
        void setUserSemantic(String const& name, UInt index);

        /// Set the (optional) known stage for this variable.
        void setStage(Stage stage);

        /// Clone all of the layout information from the `other` layout, except for offsets.
        ///
        /// This is convenience when one wants to build a variable layout "like that other one,
        /// but..."
        void cloneEverythingButOffsetsFrom(IRVarLayout* other);

        /// Build a variable layout using the current state that has been set.
        IRVarLayout* build();

    private:
        IRBuilder* m_irBuilder;
        IRBuilder* getIRBuilder() { return m_irBuilder; };

        IRTypeLayout* m_typeLayout = nullptr;
        IRVarLayout* m_pendingVarLayout = nullptr;

        IRSystemValueSemanticAttr* m_systemValueSemantic = nullptr;
        IRUserSemanticAttr* m_userSemantic = nullptr;
        IRStageAttr* m_stageAttr = nullptr;

        ResInfo m_resInfos[SLANG_PARAMETER_CATEGORY_COUNT];
    };
};

bool isVaryingResourceKind(LayoutResourceKind kind);
bool isVaryingParameter(IRTypeLayout* typeLayout);
bool isVaryingParameter(IRVarLayout* varLayout);

/// Associate layout information with an instruction.
///
/// This decoration is used in three main ways:
///
/// * To attach an `IRVarLayout` to an `IRGlobalParam` or entry-point `IRParam` representing a
/// shader parameter
/// * To attach an `IREntryPointLayout` to an `IRFunc` representing an entry point
/// * To attach an `IRTaggedUnionTypeLayout` to an `IRTaggedUnionType`
///
FIDDLE()
struct IRLayoutDecoration : IRDecoration
{
    FIDDLE(leafInst())

    /// Get the layout that is being attached to the parent instruction
    IRLayout* getLayout() { return cast<IRLayout>(getOperand(0)); }
};

//
FIDDLE()
struct IRAlignOf : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBaseOp() { return getOperand(0); }
};

FIDDLE()
struct IRCall : IRInst
{
    FIDDLE(leafInst())

    IRInst* getCallee() { return getOperand(0); }
    IRUse* getCalleeUse() { return getOperands(); }
    UInt getArgCount() { return getOperandCount() - 1; }
    IRUse* getArgs() { return getOperands() + 1; }
    IROperandList<IRInst> getArgsList()
    {
        return IROperandList<IRInst>(getOperands() + 1, getOperands() + getOperandCount());
    }
    IRInst* getArg(UInt index) { return getOperand(index + 1); }
    void setArg(UInt index, IRInst* arg) { setOperand(index + 1, arg); }
};

FIDDLE()
struct IRAlignedAttr : IRAttr
{
    FIDDLE(leafInst())
    IRInst* getAlignment() { return getOperand(0); }
};

FIDDLE()
struct IRLoad : IRInst
{
    FIDDLE(leafInst())
    IRUse ptr;

    IRInst* getPtr() { return ptr.get(); }
};

FIDDLE()
struct IRAtomicOperation : IRInst
{
    FIDDLE(baseInst())

    IRInst* getPtr() { return getOperand(0); }
};

FIDDLE()
struct IRAtomicLoad : IRAtomicOperation
{
    FIDDLE(leafInst())
    IRUse ptr;

    IRInst* getPtr() { return ptr.get(); }
};

FIDDLE()
struct IRStore : IRInst
{
    FIDDLE(leafInst())
    IRUse ptr;
    IRUse val;

    IRInst* getPtr() { return ptr.get(); }
    IRInst* getVal() { return val.get(); }

    IRUse* getPtrUse() { return &ptr; }
    IRUse* getValUse() { return &val; }
};

FIDDLE()
struct IRAtomicStore : IRAtomicOperation
{
    FIDDLE(leafInst())
    IRUse ptr;
    IRUse val;

    IRInst* getPtr() { return ptr.get(); }
    IRInst* getVal() { return val.get(); }
};

FIDDLE()
struct IRRWStructuredBufferStore : IRInst
{
    FIDDLE(leafInst())
    IRInst* getStructuredBuffer() { return getOperand(0); }
    IRInst* getIndex() { return getOperand(1); }
    IRInst* getVal() { return getOperand(2); }
};

FIDDLE()
struct IRFieldExtract : IRInst
{
    FIDDLE(leafInst())
    IRUse base;
    IRUse field;

    IRInst* getBase() { return base.get(); }
    IRInst* getField() { return field.get(); }
};

FIDDLE()
struct IRFieldAddress : IRInst
{
    FIDDLE(leafInst())
    IRUse base;
    IRUse field;

    IRInst* getBase() { return base.get(); }
    IRInst* getField() { return field.get(); }
};

FIDDLE()
struct IRGetElement : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBase() { return getOperand(0); }
    IRInst* getIndex() { return getOperand(1); }
};

FIDDLE()
struct IRGetElementPtr : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBase() { return getOperand(0); }
    IRInst* getIndex() { return getOperand(1); }
};

FIDDLE()
struct IRGetOffsetPtr : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBase() { return getOperand(0); }
    IRInst* getOffset() { return getOperand(1); }
};

FIDDLE()
struct IRRWStructuredBufferGetElementPtr : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBase() { return getOperand(0); }
    IRInst* getIndex() { return getOperand(1); }
};

FIDDLE()
struct IRStructuredBufferAppend : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBuffer() { return getOperand(0); }
    IRInst* getElement() { return getOperand(1); }
};

FIDDLE()
struct IRStructuredBufferConsume : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBuffer() { return getOperand(0); }
};

FIDDLE()
struct IRStructuredBufferGetDimensions : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBuffer() { return getOperand(0); }
};

FIDDLE()
struct IRNonUniformResourceIndex : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRLoadReverseGradient : IRInst
{
    FIDDLE(leafInst())
    IRInst* getValue() { return getOperand(0); }
};

FIDDLE()
struct IRReverseGradientDiffPairRef : IRInst
{
    FIDDLE(leafInst())
    IRInst* getPrimal() { return getOperand(0); }
    IRInst* getDiff() { return getOperand(1); }
};

FIDDLE()
struct IRPrimalParamRef : IRInst
{
    FIDDLE(leafInst())
    IRInst* getReferencedParam() { return getOperand(0); }
};

FIDDLE()
struct IRDiffParamRef : IRInst
{
    FIDDLE(leafInst())
    IRInst* getReferencedParam() { return getOperand(0); }
};

FIDDLE()
struct IRGetNativePtr : IRInst
{
    FIDDLE(leafInst())
    IRInst* getElementType() { return getOperand(0); }
};

FIDDLE()
struct IRGetManagedPtrWriteRef : IRInst
{
    FIDDLE(leafInst())
    IRInst* getPtrToManagedPtr() { return getOperand(0); }
};

FIDDLE()
struct IRGetAddress : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRImageSubscript : IRInst
{
    FIDDLE(leafInst())
    IRInst* getImage() { return getOperand(0); }
    IRInst* getCoord() { return getOperand(1); }
    bool hasSampleCoord() { return getOperandCount() > 2 && getOperand(2) != nullptr; }
    IRInst* getSampleCoord() { return getOperand(2); }
};

FIDDLE()
struct IRImageLoad : IRInst
{
    FIDDLE(leafInst())
    IRInst* getImage() { return getOperand(0); }
    IRInst* getCoord() { return getOperand(1); }

    /// If GLSL/SPIR-V, Sample coord
    /// If Metal, Array or Sample coord
    bool hasAuxCoord1() { return getOperandCount() > 2 && getOperand(2) != nullptr; }
    IRInst* getAuxCoord1() { return getOperand(2); }

    /// If Metal, Sample coord
    bool hasAuxCoord2() { return getOperandCount() > 3 && getOperand(3) != nullptr; }
    IRInst* getAuxCoord2() { return getOperand(3); }
};

FIDDLE()
struct IRImageStore : IRInst
{
    FIDDLE(leafInst())
    IRInst* getImage() { return getOperand(0); }
    IRInst* getCoord() { return getOperand(1); }
    IRInst* getValue() { return getOperand(2); }

    /// If GLSL/SPIR-V, Sample coord
    /// Metal array/face index
    bool hasAuxCoord1() { return getOperandCount() > 3 && getOperand(3) != nullptr; }
    IRInst* getAuxCoord1() { return getOperand(3); }
};
// Terminators

FIDDLE()
struct IRSelect : IRInst
{
    FIDDLE(leafInst());

    IRInst* getCondition() { return getOperand(0); }
    IRInst* getTrueResult() { return getOperand(1); }
    IRInst* getFalseResult() { return getOperand(2); }
};


FIDDLE()
struct IRReturn : IRTerminatorInst
{
    FIDDLE(leafInst())

    IRInst* getVal() { return getOperand(0); }
};

FIDDLE()
struct IRYield : IRTerminatorInst
{
    FIDDLE(leafInst())

    IRInst* getVal() { return getOperand(0); }
};

FIDDLE()
struct IRDiscard : IRTerminatorInst
{
    FIDDLE(leafInst())
};

// Used for representing a distinct copy of an object.
// This will get lowered into a no-op in the backend,
// but is useful for IR transformations that need to consider
// different uses of an inst separately.
//
// For example, when we hoist primal insts out of a loop,
// we need to make distinct copies of the inst for its uses
// within the loop body and outside of it.
//
FIDDLE()
struct IRCheckpointObject : IRInst
{
    FIDDLE(leafInst())

    IRInst* getVal() { return getOperand(0); }
};

FIDDLE()
struct IRLoopExitValue : IRInst
{
    FIDDLE(leafInst())

    IRInst* getVal() { return getOperand(0); }
};

// Signals that this point in the code should be unreachable.
// We can/should emit a dataflow error if we can ever determine
// that a block ending in one of these can actually be
// executed.
FIDDLE()
struct IRUnreachable : IRTerminatorInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMissingReturn : IRUnreachable
{
    FIDDLE(leafInst())
};

struct IRBlock;

FIDDLE()
struct IRUnconditionalBranch : IRTerminatorInst
{
    FIDDLE(baseInst())
    IRUse block;

    IRBlock* getTargetBlock() { return (IRBlock*)block.get(); }

    UInt getArgCount();
    IRUse* getArgs();
    IRInst* getArg(UInt index);
    void removeArgument(UInt index);
};

// The start of a loop is a special control-flow
// instruction, that records relevant information
// about the loop structure:
FIDDLE()
struct IRLoop : IRUnconditionalBranch
{
    FIDDLE(leafInst())
    // The next block after the loop, which
    // is where we expect control flow to
    // re-converge, and also where a
    // `break` will target.
    IRUse breakBlock;

    // The block where control flow will go
    // on a `continue`.
    IRUse continueBlock;

    IRBlock* getBreakBlock() { return (IRBlock*)breakBlock.get(); }
    IRBlock* getContinueBlock() { return (IRBlock*)continueBlock.get(); }
};

FIDDLE()
struct IRConditionalBranch : IRTerminatorInst
{
    FIDDLE(baseInst())

    IRUse condition;
    IRUse trueBlock;
    IRUse falseBlock;

    IRInst* getCondition() { return condition.get(); }
    IRBlock* getTrueBlock() { return (IRBlock*)trueBlock.get(); }
    IRBlock* getFalseBlock() { return (IRBlock*)falseBlock.get(); }
};

// A conditional branch that represents a two-sided `if`:
//
//     if( <condition> ) { <trueBlock> }
//     else              { <falseBlock> }
//     <afterBlock>
//
FIDDLE()
struct IRIfElse : IRConditionalBranch
{
    FIDDLE(leafInst())
    IRUse afterBlock;

    IRBlock* getAfterBlock() { return (IRBlock*)afterBlock.get(); }
};

// A multi-way branch that represents a source-level `switch`
FIDDLE()
struct IRSwitch : IRTerminatorInst
{
    FIDDLE(leafInst())

    IRUse condition;
    IRUse breakLabel;
    IRUse defaultLabel;

    IRInst* getCondition() { return condition.get(); }
    IRBlock* getBreakLabel() { return (IRBlock*)breakLabel.get(); }
    IRBlock* getDefaultLabel() { return (IRBlock*)defaultLabel.get(); }

    // remaining args are: caseVal, caseLabel, ...

    UInt getCaseCount() { return (getOperandCount() - 3) / 2; }
    IRInst* getCaseValue(UInt index) { return getOperand(3 + index * 2 + 0); }
    IRBlock* getCaseLabel(UInt index) { return (IRBlock*)getOperand(3 + index * 2 + 1); }
    IRUse* getCaseLabelUse(UInt index) { return getOperands() + 3 + index * 2 + 1; }
};

// A compile-time switch based on the current code generation target.
FIDDLE()
struct IRTargetSwitch : IRTerminatorInst
{
    FIDDLE(leafInst())
    IRInst* getBreakBlock() { return getOperand(0); }
    UInt getCaseCount() { return (getOperandCount() - 1) / 2; }
    IRBlock* getCaseBlock(UInt index) { return (IRBlock*)getOperand(index * 2 + 2); }
    IRInst* getCaseValue(UInt index) { return getOperand(index * 2 + 1); }
};

FIDDLE()
struct IRThrow : IRTerminatorInst
{
    FIDDLE(leafInst())

    IRInst* getValue() { return getOperand(0); }
};

FIDDLE()
struct IRTryCall : IRTerminatorInst
{
    FIDDLE(leafInst())

    IRBlock* getSuccessBlock() { return cast<IRBlock>(getOperand(0)); }
    IRBlock* getFailureBlock() { return cast<IRBlock>(getOperand(1)); }
    IRInst* getCallee() { return getOperand(2); }
    UInt getArgCount() { return getOperandCount() - 3; }
    IRUse* getArgs() { return getOperands() + 3; }
    IRInst* getArg(UInt index) { return getOperand(index + 3); }
};

FIDDLE()
struct IRDefer : IRTerminatorInst
{
    FIDDLE(leafInst())

    IRBlock* getDeferBlock() { return cast<IRBlock>(getOperand(0)); }
    IRBlock* getMergeBlock() { return cast<IRBlock>(getOperand(1)); }
    IRBlock* getScopeBlock() { return cast<IRBlock>(getOperand(2)); }
};

FIDDLE()
struct IRSwizzle : IRInst
{
    FIDDLE(leafInst())
    IRUse base;

    IRInst* getBase() { return base.get(); }
    UInt getElementCount() { return getOperandCount() - 1; }
    IRInst* getElementIndex(UInt index) { return getOperand(index + 1); }
};

FIDDLE()
struct IRSwizzleSet : IRInst
{
    FIDDLE(leafInst())
    IRUse base;
    IRUse source;

    IRInst* getBase() { return base.get(); }
    IRInst* getSource() { return source.get(); }
    UInt getElementCount() { return getOperandCount() - 2; }
    IRInst* getElementIndex(UInt index) { return getOperand(index + 2); }
};

FIDDLE()
struct IRSwizzledStore : IRInst
{
    FIDDLE(leafInst())
    IRInst* getDest() { return getOperand(0); }
    IRInst* getSource() { return getOperand(1); }
    UInt getElementCount() { return getOperandCount() - 2; }
    IRInst* getElementIndex(UInt index) { return getOperand(index + 2); }
};


FIDDLE()
struct IRPatchConstantFuncDecoration : IRDecoration
{
    FIDDLE(leafInst())

    IRInst* getFunc() { return getOperand(0); }
};

// An IR `var` instruction conceptually represents
// a stack allocation of some memory.
FIDDLE()
struct IRVar : IRInst
{
    FIDDLE(leafInst())
    IRPtrType* getDataType() { return cast<IRPtrType>(IRInst::getDataType()); }
};

/// @brief A global variable.
///
/// Represents a global variable in the IR.
/// If the variable has an initializer, then
/// it is represented by the code in the basic
/// blocks nested inside this value.
FIDDLE()
struct IRGlobalVar : IRGlobalValueWithCode
{
    FIDDLE(leafInst())

    IRPtrType* getDataType() { return cast<IRPtrType>(IRInst::getDataType()); }
};

/// @brief A global shader parameter.
///
/// Represents a uniform (as opposed to varying) shader parameter
/// passed at the global scope (entry-point `uniform` parameters
/// are encoded as ordinary function parameters.
///
/// Note that an `IRGlobalParam` directly represents the value of
/// the parameter, unlike an `IRGlobalVar`, which represents the
/// *address* of the value. As a result, global parameters are
/// immutable, and subject to various SSA simplifications that
/// do not work for global variables.
///
FIDDLE()
struct IRGlobalParam : IRInst
{
    FIDDLE(leafInst())
};

/// @brief A global constnat.
///
/// Represents a global constant that may have a name and linkage.
/// If it has an operand, then this operand is the value of
/// the constants. If there is no operand, then the instruction
/// represents an "extern" constant that will be defined in another
/// module, and which is thus expected to have linkage.
///
FIDDLE()
struct IRGlobalConstant : IRInst
{
    FIDDLE(leafInst())

    /// Get the value of this global constant, or null if the value is not known.
    IRInst* getValue() { return getOperandCount() != 0 ? getOperand(0) : nullptr; }
};

// An entry in a witness table (see below)
FIDDLE()
struct IRWitnessTableEntry : IRInst
{
    FIDDLE(leafInst())
    // The AST-level requirement
    IRUse requirementKey;

    // The IR-level value that satisfies the requirement
    IRUse satisfyingVal;

    IRInst* getRequirementKey() { return getOperand(0); }
    IRInst* getSatisfyingVal() { return getOperand(1); }
};

// A witness table is a global value that stores
// information about how a type conforms to some
// interface. It basically takes the form of a
// map from the required members of the interface
// to the IR values that satisfy those requirements.
FIDDLE()
struct IRWitnessTable : IRInst
{
    FIDDLE(leafInst())
    IRInstList<IRWitnessTableEntry> getEntries()
    {
        return IRInstList<IRWitnessTableEntry>(getChildren());
    }

    IRInst* getConformanceType()
    {
        return cast<IRWitnessTableType>(getDataType())->getConformanceType();
    }

    IRType* getConcreteType() { return (IRType*)getOperand(0); }
};

/// Represents an RTTI object.
/// An IRRTTIObject has 1 operand, specifying the type
/// this RTTI object provides info for.
/// All type info are encapsualted as `IRRTTI*Decoration`s attached
/// to the object.
FIDDLE()
struct IRRTTIObject : IRInst
{
    FIDDLE(leafInst())
};

// An instruction that yields an undefined value.
//
// Note that we make this an instruction rather than a value,
// so that we will be able to identify a variable that is
// used when undefined.
FIDDLE()
struct IRUndefined : IRInst
{
    FIDDLE(leafInst())
};

// Special inst for targets that support default initialization,
// like the braces '= {}' in C/HLSL
FIDDLE()
struct IRDefaultConstruct : IRInst
{
    FIDDLE(leafInst())
};

// A global-scope generic parameter (a type parameter, a
// constraint parameter, etc.)
FIDDLE()
struct IRGlobalGenericParam : IRInst
{
    FIDDLE(leafInst())
};

// An instruction that binds a global generic parameter
// to a particular value.
FIDDLE()
struct IRBindGlobalGenericParam : IRInst
{
    FIDDLE(leafInst())
    IRGlobalGenericParam* getParam() { return cast<IRGlobalGenericParam>(getOperand(0)); }
    IRInst* getVal() { return getOperand(1); }
};

FIDDLE()
struct IRExpand : IRInst
{
    FIDDLE(leafInst())
    UInt getCaptureCount() { return getOperandCount(); }
    IRInst* getCapture(UInt index) { return getOperand(index); }
    IRInstList<IRBlock> getBlocks() { return IRInstList<IRBlock>(getChildren()); }
};


FIDDLE()
struct IREach : IRInst
{
    FIDDLE(leafInst())

    IRInst* getElement() { return getOperand(0); }
};

FIDDLE()
struct IRMakeArray : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMakeArrayFromElement : IRInst
{
    FIDDLE(leafInst())
};


// An Instruction that creates a tuple value.
FIDDLE()
struct IRMakeTuple : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMakeValuePack : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMakeStruct : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMakeWitnessPack : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRGetTupleElement : IRInst
{
    FIDDLE(leafInst())
    IRInst* getTuple() { return getOperand(0); }
    IRInst* getElementIndex() { return getOperand(1); }
};

FIDDLE()
struct IRGetTargetTupleElement : IRInst
{
    FIDDLE(leafInst())
    IRInst* getTuple() { return getOperand(0); }
    IRInst* getElementIndex() { return getOperand(1); }
};

FIDDLE()
struct IRMakeVector : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMakeVectorFromScalar : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRMakeCoopVector : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRCoopMatMapElementIFunc : IRInst
{
    FIDDLE(leafInst())
    IRInst* getCoopMat() { return getOperand(0); }
    IRInst* getTuple() { return getOperand(0); }
    IRFunc* getIFuncCall() { return as<IRFunc>(getOperand(1)); }
    IRInst* getIFuncThis() { return getOperand(2); }

    bool hasIFuncThis() { return getOperandCount() > 2; }
    void setIFuncCall(IRFunc* func) { setOperand(1, func); }
};

// An Instruction that creates a differential pair value from a
// primal and differential.

FIDDLE()
struct IRMakeDifferentialPairBase : IRInst
{
    FIDDLE(baseInst())
    IRInst* getPrimalValue() { return getOperand(0); }
    IRInst* getDifferentialValue() { return getOperand(1); }
};
FIDDLE()
struct IRMakeDifferentialPair : IRMakeDifferentialPairBase
{
    FIDDLE(leafInst())
};
FIDDLE()
struct IRMakeDifferentialPairUserCode : IRMakeDifferentialPairBase
{
    FIDDLE(leafInst())
};
FIDDLE()
struct IRMakeDifferentialPtrPair : IRMakeDifferentialPairBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRDifferentialPairGetDifferentialBase : IRInst
{
    FIDDLE(baseInst())
    IRInst* getBase() { return getOperand(0); }
};
FIDDLE()
struct IRDifferentialPairGetDifferential : IRDifferentialPairGetDifferentialBase
{
    FIDDLE(leafInst())
};
FIDDLE()
struct IRDifferentialPairGetDifferentialUserCode : IRDifferentialPairGetDifferentialBase
{
    FIDDLE(leafInst())
};
FIDDLE()
struct IRDifferentialPtrPairGetDifferential : IRDifferentialPairGetDifferentialBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRDifferentialPairGetPrimalBase : IRInst
{
    FIDDLE(baseInst())
    IRInst* getBase() { return getOperand(0); }
};
FIDDLE()
struct IRDifferentialPairGetPrimal : IRDifferentialPairGetPrimalBase
{
    FIDDLE(leafInst())
};
FIDDLE()
struct IRDifferentialPairGetPrimalUserCode : IRDifferentialPairGetPrimalBase
{
    FIDDLE(leafInst())
};
FIDDLE()
struct IRDifferentialPtrPairGetPrimal : IRDifferentialPairGetPrimalBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRDetachDerivative : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBase() { return getOperand(0); }
};

FIDDLE()
struct IRUpdateElement : IRInst
{
    FIDDLE(leafInst())

    IRInst* getOldValue() { return getOperand(0); }
    IRInst* getElementValue() { return getOperand(1); }
    IRInst* getAccessKey(UInt index) { return getOperand(2 + index); }
    UInt getAccessKeyCount() { return getOperandCount() - 2; }
    List<IRInst*> getAccessChain()
    {
        List<IRInst*> result;
        for (UInt i = 0; i < getAccessKeyCount(); i++)
            result.add(getAccessKey(i));
        return result;
    }
};

// Constructs an `Result<T,E>` value from an error code.
FIDDLE()
struct IRMakeResultError : IRInst
{
    FIDDLE(leafInst())

    IRInst* getErrorValue() { return getOperand(0); }
};

// Constructs an `Result<T,E>` value from an valid value.
FIDDLE()
struct IRMakeResultValue : IRInst
{
    FIDDLE(leafInst())

    IRInst* getValue() { return getOperand(0); }
};

// Determines if a `Result` value represents an error.
FIDDLE()
struct IRIsResultError : IRInst
{
    FIDDLE(leafInst())

    IRInst* getResultOperand() { return getOperand(0); }
};

// Extract the value from a `Result`.
FIDDLE()
struct IRGetResultValue : IRInst
{
    FIDDLE(leafInst())

    IRInst* getResultOperand() { return getOperand(0); }
};

// Extract the error code from a `Result`.
FIDDLE()
struct IRGetResultError : IRInst
{
    FIDDLE(leafInst())

    IRInst* getResultOperand() { return getOperand(0); }
};

FIDDLE()
struct IROptionalHasValue : IRInst
{
    FIDDLE(leafInst())

    IRInst* getOptionalOperand() { return getOperand(0); }
};

FIDDLE()
struct IRGetOptionalValue : IRInst
{
    FIDDLE(leafInst())

    IRInst* getOptionalOperand() { return getOperand(0); }
};

FIDDLE()
struct IRMakeOptionalValue : IRInst
{
    FIDDLE(leafInst())

    IRInst* getValue() { return getOperand(0); }
};

FIDDLE()
struct IRMakeOptionalNone : IRInst
{
    FIDDLE(leafInst())
    IRInst* getDefaultValue() { return getOperand(0); }
};

/// An instruction that packs a concrete value into an existential-type "box"
FIDDLE()
struct IRMakeExistential : IRInst
{
    FIDDLE(leafInst())
    IRInst* getWrappedValue() { return getOperand(0); }
    IRInst* getWitnessTable() { return getOperand(1); }
};

FIDDLE()
struct IRMakeExistentialWithRTTI : IRInst
{
    FIDDLE(leafInst())
    IRInst* getWrappedValue() { return getOperand(0); }
    IRInst* getWitnessTable() { return getOperand(1); }
    IRInst* getRTTI() { return getOperand(2); }
};

FIDDLE()
struct IRCreateExistentialObject : IRInst
{
    FIDDLE(leafInst())
    IRInst* getTypeID() { return getOperand(0); }
    IRInst* getValue() { return getOperand(1); }
};

/// Generalizes `IRMakeExistential` by allowing a type with existential sub-fields to be boxed
FIDDLE()
struct IRWrapExistential : IRInst
{
    FIDDLE(leafInst())
    IRInst* getWrappedValue() { return getOperand(0); }

    UInt getSlotOperandCount() { return getOperandCount() - 1; }
    IRInst* getSlotOperand(UInt index) { return getOperand(index + 1); }
    IRUse* getSlotOperands() { return getOperands() + 1; }
};

FIDDLE()
struct IRGetValueFromBoundInterface : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRExtractExistentialValue : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRExtractExistentialType : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRExtractExistentialWitnessTable : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRIsNullExistential : IRInst
{
    FIDDLE(leafInst())
};


/* Base class for instructions that track liveness */
FIDDLE()
struct IRLiveRangeMarker : IRInst
{
    FIDDLE(baseInst())

    // TODO(JS): It might be useful to track how many bytes are live in the item referenced.
    // It's not entirely clear how that will work across different targets, or even what such a
    // size means on some targets.
    //
    // Here we assume the size is the size of the type being referenced (whatever that means on a
    // target)
    //
    // Potentially we could have a count, for defining (say) a range of an array. It's not clear
    // this is needed, so we just have the item referenced.

    /// The referenced item whose liveness starts after this instruction
    IRInst* getReferenced() { return getOperand(0); }
};

/// Identifies then the item references starts being live.
FIDDLE()
struct IRLiveRangeStart : IRLiveRangeMarker
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRIsType : IRInst
{
    FIDDLE(leafInst())

    IRInst* getValue() { return getOperand(0); }
    IRInst* getValueWitness() { return getOperand(1); }

    IRInst* getTypeOperand() { return getOperand(2); }
    IRInst* getTargetWitness() { return getOperand(3); }
};

/// Demarks where the referenced item is no longer live, optimimally (although not
/// necessarily) at the previous instruction.
///
/// There *can* be acceses to the referenced item after the end, if those accesses
/// can never be seen. For example if there is a store, without any subsequent loads,
/// the store will never be seen (by a load) and so can be ignored.
///
/// In general there can be one or more 'ends' for every start.
FIDDLE()
struct IRLiveRangeEnd : IRLiveRangeMarker
{
    FIDDLE(leafInst())
};

/// An instruction that queries binding information about an opaque/resource value.
///
FIDDLE()
struct IRBindingQuery : IRInst
{
    FIDDLE(baseInst())

    IRInst* getOpaqueValue() { return getOperand(0); }
};

FIDDLE()
struct IRGetRegisterIndex : IRBindingQuery
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRGetRegisterSpace : IRBindingQuery
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRIntCast : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRFloatCast : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRCastIntToFloat : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRCastFloatToInt : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRDebugSource : IRInst
{
    FIDDLE(leafInst())
    IRInst* getFileName() { return getOperand(0); }
    IRInst* getSource() { return getOperand(1); }
};

FIDDLE()
struct IRDebugBuildIdentifier : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBuildIdentifier() { return getOperand(0); }
    IRInst* getFlags() { return getOperand(1); }
};

FIDDLE()
struct IRDebugLine : IRInst
{
    FIDDLE(leafInst())
    IRInst* getSource() { return getOperand(0); }
    IRInst* getLineStart() { return getOperand(1); }
    IRInst* getLineEnd() { return getOperand(2); }
    IRInst* getColStart() { return getOperand(3); }
    IRInst* getColEnd() { return getOperand(4); }
};

FIDDLE()
struct IRDebugVar : IRInst
{
    FIDDLE(leafInst())
    IRInst* getSource() { return getOperand(0); }
    IRInst* getLine() { return getOperand(1); }
    IRInst* getCol() { return getOperand(2); }
    IRInst* getArgIndex() { return getOperandCount() >= 4 ? getOperand(3) : nullptr; }
};

FIDDLE()
struct IRDebugValue : IRInst
{
    FIDDLE(leafInst())
    IRInst* getDebugVar() { return getOperand(0); }
    IRInst* getValue() { return getOperand(1); }
};

FIDDLE()
struct IRDebugInlinedAt : IRInst
{
    FIDDLE(leafInst())
    IRInst* getLine() { return getOperand(0); }
    IRInst* getCol() { return getOperand(1); }
    IRInst* getFile() { return getOperand(2); }
    IRInst* getDebugFunc() { return getOperand(3); }
    IRInst* getOuterInlinedAt()
    {
        if (operandCount == 5)
            return getOperand(4);
        return nullptr;
    }
    void setDebugFunc(IRInst* func) { setOperand(3, func); }
    bool isOuterInlinedPresent() { return operandCount == 5; }
};

FIDDLE()
struct IRDebugScope : IRInst
{
    FIDDLE(leafInst())
    IRInst* getScope() { return getOperand(0); }
    IRInst* getInlinedAt() { return getOperand(1); }
    void setInlinedAt(IRInst* inlinedAt) { setOperand(1, inlinedAt); }
};

FIDDLE()
struct IRDebugNoScope : IRInst
{
    FIDDLE(leafInst())
    IRInst* getScope() { return getOperand(0); }
};

FIDDLE()
struct IRDebugInlinedVariable : IRInst
{
    FIDDLE(leafInst())
    IRInst* getVariable() { return getOperand(0); }
    IRInst* getInlinedAt() { return getOperand(1); }
};

FIDDLE()
struct IRDebugFunction : IRInst
{
    FIDDLE(leafInst())
    IRInst* getName() { return getOperand(0); }
    IRInst* getLine() { return getOperand(1); }
    IRInst* getCol() { return getOperand(2); }
    IRInst* getFile() { return getOperand(3); }
    IRInst* getDebugType() { return getOperand(4); }
};

FIDDLE()
struct IRDebugFuncDecoration : IRInst
{
    FIDDLE(leafInst())
    IRInst* getDebugFunc() { return getOperand(0); }
};

FIDDLE()
struct IRDebugLocationDecoration : IRDecoration
{
    FIDDLE(leafInst())
    IRInst* getSource() { return getOperand(0); }
    IRInst* getLine() { return getOperand(1); }
    IRInst* getCol() { return getOperand(2); }
};

struct IRSPIRVAsm;

FIDDLE()
struct IRSPIRVAsmOperand : IRInst
{
    FIDDLE(baseInst())
    IRInst* getValue()
    {
        if (getOp() == kIROp_SPIRVAsmOperandResult)
            return nullptr;
        return getOperand(0);
    }
    IRSPIRVAsm* getAsmBlock()
    {
        const auto ret = as<IRSPIRVAsm>(getParent());
        SLANG_ASSERT(ret);
        return ret;
    }
};

FIDDLE()
struct IRSPIRVAsmOperandInst : IRSPIRVAsmOperand
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRSPIRVAsmInst : IRInst
{
    FIDDLE(leafInst())

    IRSPIRVAsmOperand* getOpcodeOperand()
    {
        auto operand = getOperand(0);
        if (auto globalRef = as<IRGlobalValueRef>(operand))
            operand = globalRef->getValue();
        const auto opcodeOperand = cast<IRSPIRVAsmOperand>(operand);
        // This must be either:
        // - An enum, such as 'OpNop'
        // - The __truncate pseudo-instruction
        // - A literal, like 107 (OpImageQuerySamples)
        SLANG_ASSERT(
            opcodeOperand->getOp() == kIROp_SPIRVAsmOperandEnum ||
            opcodeOperand->getOp() == kIROp_SPIRVAsmOperandTruncate ||
            opcodeOperand->getOp() == kIROp_SPIRVAsmOperandLiteral);
        return opcodeOperand;
    }

    SpvWord getOpcodeOperandWord()
    {
        const auto o = getOpcodeOperand();
        const auto v = o->getValue();
        // It's not valid to call this on an operand which doesn't have a value
        // (such as __truncate)
        SLANG_ASSERT(v);
        const auto i = cast<IRIntLit>(v);
        return SpvWord(i->getValue());
    }

    IROperandList<IRSPIRVAsmOperand> getSPIRVOperands()
    {
        return IROperandList<IRSPIRVAsmOperand>(
            getOperands() + 1,
            getOperands() + getOperandCount());
    }
};

FIDDLE()
struct IRSPIRVAsm : IRInst
{
    FIDDLE(leafInst())
    IRFilteredInstList<IRSPIRVAsmInst> getInsts()
    {
        return IRFilteredInstList<IRSPIRVAsmInst>(getFirstChild(), getLastChild());
    }
};

FIDDLE()
struct IRGenericAsm : IRTerminatorInst
{
    FIDDLE(leafInst())
    UnownedStringSlice getAsm() { return as<IRStringLit>(getOperand(0))->getStringSlice(); }
};

FIDDLE()
struct IRRequirePrelude : IRInst
{
    FIDDLE(leafInst())
    UnownedStringSlice getPrelude() { return as<IRStringLit>(getOperand(0))->getStringSlice(); }
};

FIDDLE()
struct IRRequireTargetExtension : IRInst
{
    FIDDLE(leafInst())
    UnownedStringSlice getExtensionName()
    {
        return as<IRStringLit>(getOperand(0))->getStringSlice();
    }
};

FIDDLE()
struct IRRequireComputeDerivative : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRRequireMaximallyReconverges : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRRequireQuadDerivatives : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRStaticAssert : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IREmbeddedDownstreamIR : IRInst
{
    FIDDLE(leafInst())
    CodeGenTarget getTarget()
    {
        return static_cast<CodeGenTarget>(cast<IRIntLit>(getOperand(0))->getValue());
    }
    IRBlobLit* getBlob() { return cast<IRBlobLit>(getOperand(1)); }
};

FIDDLE(allOtherInstStructs())

struct IRBuilderSourceLocRAII;

struct IRBuilder
{
private:
    /// Deduplication context from the module.
    IRDeduplicationContext* m_dedupContext = nullptr;

    IRModule* m_module = nullptr;

    /// Default location for inserting new instructions as they are emitted
    IRInsertLoc m_insertLoc;

    /// Information that controls how source locations are associatd with instructions that get
    /// emitted
    IRBuilderSourceLocRAII* m_sourceLocInfo = nullptr;

public:
    IRBuilder() {}

    explicit IRBuilder(IRModule* module)
        : m_module(module), m_dedupContext(module->getDeduplicationContext())
    {
    }

    explicit IRBuilder(IRInst* inst)
        : m_module(inst->getModule()), m_dedupContext(inst->getModule()->getDeduplicationContext())
    {
    }

    Session* getSession() const { return m_module->getSession(); }

    IRModule* getModule() const { return m_module; }

    IRInsertLoc const& getInsertLoc() const { return m_insertLoc; }

    void setInsertLoc(IRInsertLoc const& loc) { m_insertLoc = loc; }

    // Get the current basic block we are inserting into (if any)
    IRBlock* getBlock() { return m_insertLoc.getBlock(); }

    // Get the current function (or other value with code)
    // that we are inserting into (if any).
    IRInst* getFunc() { return m_insertLoc.getFunc(); }

    void setInsertInto(IRInst* insertInto) { setInsertLoc(IRInsertLoc::atEnd(insertInto)); }
    void setInsertBefore(IRInst* insertBefore) { setInsertLoc(IRInsertLoc::before(insertBefore)); }
    // TODO: Ellie, contrary to IRInsertLoc::after, this inserts instructions in the order they are
    // emitted, should it have a better name (setInsertBeforeNext)?
    void setInsertAfter(IRInst* insertAfter);

    void setInsertInto(IRModule* module) { setInsertInto(module->getModuleInst()); }

    IRBuilderSourceLocRAII* getSourceLocInfo() const { return m_sourceLocInfo; }
    void setSourceLocInfo(IRBuilderSourceLocRAII* sourceLocInfo)
    {
        m_sourceLocInfo = sourceLocInfo;
    }

    //
    // Low-level interface for instruction creation/insertion.
    //

    /// Either find or create an `IRConstant` that matches the value of `keyInst`.
    ///
    /// This operation will re-use an existing constant with the same type and
    /// value if one can be found (currently identified through the `SharedIRBuilder`).
    /// Otherwise it will create a new `IRConstant` with the given value and register it.
    ///
    IRConstant* _findOrEmitConstant(IRConstant& keyInst);

    /// Implements a special case of inst creation (intended only for calling from `_createInst`)
    /// that returns an matching existing hoistable inst if it exists, otherwise it creates the inst
    /// and add it to the global numbering map.
    IRInst* _findOrEmitHoistableInst(
        IRType* type,
        IROp op,
        Int fixedArgCount,
        IRInst* const* fixedArgs,
        Int varArgListCount,
        Int const* listArgCounts,
        IRInst* const* const* listArgs);

    /// Create a new instruction with the given `type` and `op`, with an allocated
    /// size of at least `minSizeInBytes`, and with its operand list initialized
    /// from the provided lists of "fixed" and "variable" operands.
    ///
    /// The `fixedArgs` array must contain `fixedArgCount` operands, and will be
    /// the initial operands in the operand list of the instruction.
    ///
    /// After the fixed arguments, the instruction may have zero or more additional
    /// lists of "variable" operands, which are all concatenated. The total number
    /// of such additional lists is given by `varArgsListCount`. The number of
    /// operands in list `i` is given by `listArgCounts[i]`, and the arguments in
    /// list `i` are pointed to by `listArgs[i]`.
    ///
    /// The allocation for the instruction created will be at least `minSizeInBytes`,
    /// but may be larger if the total number of operands provided implies a larger
    /// size.
    ///
    /// Note: This is an extremely low-level operation and clients of an `IRBuilder`
    /// should not be using it when other options are available. This is also where
    /// all insts creation are bottlenecked through.
    ///
    IRInst* _createInst(
        size_t minSizeInBytes,
        IRType* type,
        IROp op,
        Int fixedArgCount,
        IRInst* const* fixedArgs,
        Int varArgListCount,
        Int const* listArgCounts,
        IRInst* const* const* listArgs);


    /// Create a new instruction with the given `type` and `op`, with an allocated
    /// size of at least `minSizeInBytes`, and with zero operands.
    ///
    IRInst* _createInst(size_t minSizeInBytes, IRType* type, IROp op)
    {
        return _createInst(minSizeInBytes, type, op, 0, nullptr, 0, nullptr, nullptr);
    }

    /// Attempt to attach a useful source location to `inst`.
    ///
    /// This operation looks at the source location information that has been
    /// attached to the builder. If it finds a valid source location, it will
    /// attach that location to `inst`.
    ///
    void _maybeSetSourceLoc(IRInst* inst);


    //

    void addInst(IRInst* inst);

    // Replace the operand of a potentially hoistable inst.
    // If the hoistable inst become duplicate of an existing inst,
    // all uses of the original user will be replaced with the existing inst.
    // The function returns the new user after any potential updates.
    IRInst* replaceOperand(IRUse* use, IRInst* newValue);

    IRInst* getBoolValue(bool value);
    IRInst* getIntValue(IRIntegerValue value);
    IRInst* getIntValue(IRType* type, IRIntegerValue value);
    IRInst* getFloatValue(IRType* type, IRFloatingPointValue value);
    IRStringLit* getStringValue(const UnownedStringSlice& slice);
    IRBlobLit* getBlobValue(ISlangBlob* blob);
    IRPtrLit* getPtrValue(IRType* type, void* ptr);
    IRPtrLit* getNullPtrValue(IRType* type);
    IRPtrLit* getNullVoidPtrValue() { return getNullPtrValue(getPtrType(getVoidType())); }
    IRVoidLit* getVoidValue();
    IRInst* getCapabilityValue(CapabilitySet const& caps);

    IRBasicType* getBasicType(BaseType baseType);
    IRBasicType* getVoidType();
    IRBasicType* getBoolType();
    IRBasicType* getIntType();
    IRBasicType* getInt64Type();
    IRBasicType* getUIntType();
    IRBasicType* getUInt64Type();
    IRBasicType* getUInt16Type();
    IRBasicType* getUInt8Type();
    IRBasicType* getFloatType();
    IRBasicType* getCharType();
    IRStringType* getStringType();
    IRNativeStringType* getNativeStringType();
    IRNativePtrType* getNativePtrType(IRType* valueType);

    IRType* getCapabilitySetType();

    IRAssociatedType* getAssociatedType(ArrayView<IRInterfaceType*> constraintTypes);
    IRThisType* getThisType(IRType* interfaceType);
    IRRawPointerType* getRawPointerType();
    IRRTTIPointerType* getRTTIPointerType(IRInst* rttiPtr);
    IRRTTIType* getRTTIType();
    IRRTTIHandleType* getRTTIHandleType();
    IRAnyValueType* getAnyValueType(IRIntegerValue size);
    IRAnyValueType* getAnyValueType(IRInst* size);
    IRDynamicType* getDynamicType();

    IRTargetTupleType* getTargetTupleType(UInt count, IRType* const* types);

    IRTupleType* getTupleType(UInt count, IRType* const* types);
    IRTupleType* getTupleType(List<IRType*> const& types)
    {
        return getTupleType(types.getCount(), types.getBuffer());
    }

    IRTupleType* getTupleType(IRType* type0, IRType* type1);
    IRTupleType* getTupleType(IRType* type0, IRType* type1, IRType* type2);
    IRTupleType* getTupleType(IRType* type0, IRType* type1, IRType* type2, IRType* type3);

    IRTypePack* getTypePack(UInt count, IRType* const* types);

    IRExpandTypeOrVal* getExpandTypeOrVal(
        IRType* type,
        IRInst* pattern,
        ArrayView<IRInst*> capture);

    IRResultType* getResultType(IRType* valueType, IRType* errorType);
    IROptionalType* getOptionalType(IRType* valueType);

    IRBasicBlockType* getBasicBlockType();
    IRWitnessTableType* getWitnessTableType(IRType* baseType);
    IRWitnessTableIDType* getWitnessTableIDType(IRType* baseType);
    IRType* getTypeType() { return getType(IROp::kIROp_TypeType); }
    IRType* getTypeParameterPackKind() { return getType(IROp::kIROp_TypeParameterPackKind); }
    IRType* getKeyType() { return nullptr; }

    IRTypeKind* getTypeKind();
    IRGenericKind* getGenericKind();

    IRPtrType* getPtrType(IRType* valueType);

    // Form a ptr type to `valueType` using the same opcode and address space as `ptrWithAddrSpace`.
    IRPtrTypeBase* getPtrTypeWithAddressSpace(IRType* valueType, IRPtrTypeBase* ptrWithAddrSpace);

    IROutType* getOutType(IRType* valueType);
    IRInOutType* getInOutType(IRType* valueType);
    IRRefType* getRefType(IRType* valueType, AddressSpace addrSpace);
    IRConstRefType* getConstRefType(IRType* valueType);
    IRPtrTypeBase* getPtrType(IROp op, IRType* valueType);
    IRPtrType* getPtrType(IROp op, IRType* valueType, AddressSpace addressSpace);
    IRPtrType* getPtrType(IROp op, IRType* valueType, IRInst* addressSpace);
    IRPtrType* getPtrType(IRType* valueType, AddressSpace addressSpace)
    {
        return getPtrType(kIROp_PtrType, valueType, addressSpace);
    }

    IRTextureTypeBase* getTextureType(
        IRType* elementType,
        IRInst* shape,
        IRInst* isArray,
        IRInst* isMS,
        IRInst* sampleCount,
        IRInst* access,
        IRInst* isShadow,
        IRInst* isCombined,
        IRInst* format);

    IRComPtrType* getComPtrType(IRType* valueType);

    /// Get a 'SPIRV literal'
    IRSPIRVLiteralType* getSPIRVLiteralType(IRType* type);

    IRArrayTypeBase* getArrayTypeBase(
        IROp op,
        IRType* elementType,
        IRInst* elementCount,
        IRInst* stride = nullptr);

    IRArrayType* getArrayType(IRType* elementType, IRInst* elementCount);

    IRUnsizedArrayType* getUnsizedArrayType(IRType* elementType);

    IRArrayType* getArrayType(IRType* elementType, IRInst* elementCount, IRInst* stride);

    IRUnsizedArrayType* getUnsizedArrayType(IRType* elementType, IRInst* stride);

    IRVectorType* getVectorType(IRType* elementType, IRInst* elementCount);

    IRVectorType* getVectorType(IRType* elementType, IRIntegerValue elementCount);

    IRCoopVectorType* getCoopVectorType(IRType* elementType, IRInst* elementCount);

    IRMatrixType* getMatrixType(
        IRType* elementType,
        IRInst* rowCount,
        IRInst* columnCount,
        IRInst* layout);

    IRArrayListType* getArrayListType(IRType* elementType);
    IRTensorViewType* getTensorViewType(IRType* elementType);
    IRTorchTensorType* getTorchTensorType(IRType* elementType);

    IRDifferentialPairType* getDifferentialPairType(IRType* valueType, IRInst* witnessTable);

    IRDifferentialPtrPairType* getDifferentialPtrPairType(IRType* valueType, IRInst* witnessTable);

    IRDifferentialPairUserCodeType* getDifferentialPairUserCodeType(
        IRType* valueType,
        IRInst* witnessTable);

    IRBackwardDiffIntermediateContextType* getBackwardDiffIntermediateContextType(IRInst* func);

    IRFuncType* getFuncType(UInt paramCount, IRType* const* paramTypes, IRType* resultType);

    IRFuncType* getFuncType(
        UInt paramCount,
        IRType* const* paramTypes,
        IRType* resultType,
        IRAttr* attribute);

    IRFuncType* getFuncType(List<IRType*> const& paramTypes, IRType* resultType)
    {
        return getFuncType(paramTypes.getCount(), paramTypes.getBuffer(), resultType);
    }

    IRConstantBufferType* getConstantBufferType(IRType* elementType, IRType* layout);

    IRGLSLOutputParameterGroupType* getGLSLOutputParameterGroupType(IRType* valueType);

    IRConstExprRate* getConstExprRate();
    IRGroupSharedRate* getGroupSharedRate();
    IRActualGlobalRate* getActualGlobalRate();
    IRSpecConstRate* getSpecConstRate();

    IRRateQualifiedType* getRateQualifiedType(IRRate* rate, IRType* dataType);

    IRType* getBindExistentialsType(IRInst* baseType, UInt slotArgCount, IRInst* const* slotArgs);

    IRType* getBindExistentialsType(IRInst* baseType, UInt slotArgCount, IRUse const* slotArgs);

    IRType* getBoundInterfaceType(
        IRType* interfaceType,
        IRType* concreteType,
        IRInst* witnessTable);

    IRType* getPseudoPtrType(IRType* concreteType);

    IRType* getConjunctionType(UInt typeCount, IRType* const* types);

    IRType* getConjunctionType(IRType* type0, IRType* type1)
    {
        IRType* types[] = {type0, type1};
        return getConjunctionType(2, types);
    }

    IRType* getAttributedType(IRType* baseType, UInt attributeCount, IRAttr* const* attributes);

    IRType* getAttributedType(IRType* baseType, List<IRAttr*> attributes)
    {
        return getAttributedType(baseType, attributes.getCount(), attributes.getBuffer());
    }

    IRInst* getIndexedFieldKey(IRInst* baseType, UInt fieldIndex)
    {
        IRInst* args[] = {baseType, getIntValue(getIntType(), fieldIndex)};
        return emitIntrinsicInst(getVoidType(), kIROp_IndexedFieldKey, 2, args);
    }

    IRMetalMeshGridPropertiesType* getMetalMeshGridPropertiesType()
    {
        return (IRMetalMeshGridPropertiesType*)getType(kIROp_MetalMeshGridPropertiesType);
    }

    IRMetalMeshType* getMetalMeshType(
        IRType* vertexType,
        IRType* primitiveType,
        IRInst* numVertices,
        IRInst* numPrimitives,
        IRInst* topology)
    {
        IRInst* ops[5] = {vertexType, primitiveType, numVertices, numPrimitives, topology};
        return (IRMetalMeshType*)getType(kIROp_MetalMeshType, 5, ops);
    }

    IRInst* emitDebugSource(UnownedStringSlice fileName, UnownedStringSlice source);
    IRInst* emitDebugBuildIdentifier(UnownedStringSlice buildIdentifier, IRIntegerValue flags);
    IRInst* emitDebugBuildIdentifier(IRInst* debugBuildIdentifier);
    IRInst* emitDebugLine(
        IRInst* source,
        IRIntegerValue lineStart,
        IRIntegerValue lineEnd,
        IRIntegerValue colStart,
        IRIntegerValue colEnd);
    IRInst* emitDebugVar(
        IRType* type,
        IRInst* source,
        IRInst* line,
        IRInst* col,
        IRInst* argIndex = nullptr);
    IRInst* emitDebugValue(IRInst* debugVar, IRInst* debugValue);
    IRInst* emitDebugInlinedAt(
        IRInst* line,
        IRInst* col,
        IRInst* file,
        IRInst* debugFunc,
        IRInst* outerInlinedAt);
    IRInst* emitDebugInlinedVariable(IRInst* variable, IRInst* inlinedAt);
    IRInst* emitDebugScope(IRInst* scope, IRInst* inlinedAt);
    IRInst* emitDebugNoScope();
    IRInst* emitDebugFunction(
        IRInst* name,
        IRInst* line,
        IRInst* col,
        IRInst* file,
        IRInst* debugType);

    /// Emit an LiveRangeStart instruction indicating the referenced item is live following this
    /// instruction
    IRLiveRangeStart* emitLiveRangeStart(IRInst* referenced);

    /// Emit a LiveRangeEnd instruction indicating the referenced item is no longer live when this
    /// instruction is reached.
    IRLiveRangeEnd* emitLiveRangeEnd(IRInst* referenced);

    // Set the data type of an instruction, while preserving
    // its rate, if any.
    void setDataType(IRInst* inst, IRType* dataType);

    IRInst* emitGetCurrentStage();

    /// Extract the value wrapped inside an existential box.
    IRInst* emitGetValueFromBoundInterface(IRType* type, IRInst* boundInterfaceValue);

    /// Given an existential value, extract the underlying "real" value
    IRInst* emitExtractExistentialValue(IRType* type, IRInst* existentialValue);

    /// Given an existential value, extract the underlying "real" type
    IRType* emitExtractExistentialType(IRInst* existentialValue);

    /// Given an existential value, return if it is empty/null.
    IRInst* emitIsNullExistential(IRInst* existentialValue);

    /// Given an existential value, extract the witness table showing how the value conforms to the
    /// existential type.
    IRInst* emitExtractExistentialWitnessTable(IRInst* existentialValue);

    IRInst* emitForwardDifferentiateInst(IRType* type, IRInst* baseFn);
    IRInst* emitBackwardDifferentiateInst(IRType* type, IRInst* baseFn);
    IRInst* emitBackwardDifferentiatePrimalInst(IRType* type, IRInst* baseFn);
    IRInst* emitBackwardDifferentiatePropagateInst(IRType* type, IRInst* baseFn);
    IRInst* emitPrimalSubstituteInst(IRType* type, IRInst* baseFn);
    IRInst* emitDetachDerivative(IRType* type, IRInst* value);
    IRInst* emitIsDifferentialNull(IRInst* value);

    IRInst* emitDispatchKernelInst(
        IRType* type,
        IRInst* baseFn,
        IRInst* threadGroupSize,
        IRInst* dispatchSize,
        Int argCount,
        IRInst* const* inArgs);
    IRInst* emitCudaKernelLaunch(
        IRInst* baseFn,
        IRInst* gridDim,
        IRInst* blockDim,
        IRInst* argsArray,
        IRInst* cudaStream);
    IRInst* emitGetTorchCudaStream();

    IRInst* emitMakeDifferentialPair(IRType* type, IRInst* primal, IRInst* differential);
    IRInst* emitMakeDifferentialValuePair(IRType* type, IRInst* primal, IRInst* differential);
    IRInst* emitMakeDifferentialPtrPair(IRType* type, IRInst* primal, IRInst* differential);
    IRInst* emitMakeDifferentialPairUserCode(IRType* type, IRInst* primal, IRInst* differential);

    IRInst* addDifferentiableTypeDictionaryDecoration(IRInst* target);

    IRInst* addPrimalValueStructKeyDecoration(IRInst* target, IRStructKey* key);
    IRInst* addPrimalElementTypeDecoration(IRInst* target, IRInst* type);
    IRInst* addIntermediateContextFieldDifferentialTypeDecoration(IRInst* target, IRInst* witness);

    // Add a differentiable type entry to the appropriate dictionary.
    IRInst* addDifferentiableTypeEntry(
        IRInst* dictDecoration,
        IRInst* irType,
        IRInst* conformanceWitness);

    IRInst* addFloatingModeOverrideDecoration(IRInst* dest, FloatingPointMode mode);

    IRInst* addNumThreadsDecoration(IRInst* inst, IRInst* x, IRInst* y, IRInst* z);
    IRInst* addFpDenormalPreserveDecoration(IRInst* inst, IRInst* width);
    IRInst* addFpDenormalFlushToZeroDecoration(IRInst* inst, IRInst* width);
    IRInst* addWaveSizeDecoration(IRInst* inst, IRInst* numLanes);

    IRInst* emitSpecializeInst(
        IRType* type,
        IRInst* genericVal,
        UInt argCount,
        IRInst* const* args);

    IRInst* emitSpecializeInst(IRType* type, IRInst* genericVal, const List<IRInst*>& args)
    {
        return emitSpecializeInst(type, genericVal, args.getCount(), args.begin());
    }

    IRInst* emitExpandInst(IRType* type, UInt capturedArgCount, IRInst* const* capturedArgs);
    IRInst* emitEachInst(IRType* type, IRInst* base, IRInst* indexArg = nullptr);

    IRInst* emitLookupInterfaceMethodInst(
        IRType* type,
        IRInst* witnessTableVal,
        IRInst* interfaceMethodVal);

    IRInst* emitGetSequentialIDInst(IRInst* rttiObj);

    IRInst* emitAlloca(IRInst* type, IRInst* rttiObjPtr);

    IRInst* emitGlobalValueRef(IRInst* globalInst);

    IRInst* emitBitfieldExtract(IRType* type, IRInst* op0, IRInst* op1, IRInst* op2);

    IRInst* emitBitfieldInsert(IRType* type, IRInst* op0, IRInst* op1, IRInst* op2, IRInst* op3);

    IRInst* emitPackAnyValue(IRType* type, IRInst* value);

    IRInst* emitUnpackAnyValue(IRType* type, IRInst* value);

    IRCall* emitCallInst(IRType* type, IRInst* func, UInt argCount, IRInst* const* args);

    IRCall* emitCallInst(IRType* type, IRInst* func, List<IRInst*> const& args)
    {
        return emitCallInst(type, func, args.getCount(), args.getBuffer());
    }
    IRCall* emitCallInst(IRType* type, IRInst* func, ArrayView<IRInst*> args)
    {
        return emitCallInst(type, func, args.getCount(), args.getBuffer());
    }

    IRInst* emitTryCallInst(
        IRType* type,
        IRBlock* successBlock,
        IRBlock* failureBlock,
        IRInst* func,
        UInt argCount,
        IRInst* const* args);

    IRInst* createIntrinsicInst(IRType* type, IROp op, UInt argCount, IRInst* const* args);

    IRInst* createIntrinsicInst(
        IRType* type,
        IROp op,
        IRInst* operand,
        UInt operandCount,
        IRInst* const* operands);

    IRInst* createIntrinsicInst(
        IRType* type,
        IROp op,
        UInt operandListCount,
        UInt const* listOperandCounts,
        IRInst* const* const* listOperands);

    IRInst* emitIntrinsicInst(IRType* type, IROp op, UInt argCount, IRInst* const* args);

    /// Emits appropriate inst for constructing a default value of `type`.
    /// If `fallback` is true, will emit `DefaultConstruct` inst on unknown types.
    /// Otherwise, returns nullptr if we can't materialize the inst.
    IRInst* emitDefaultConstruct(IRType* type, bool fallback = true);

    /// Emits a raw `DefaultConstruct` opcode without attempting to fold/materialize
    /// the inst.
    IRInst* emitDefaultConstructRaw(IRType* type);

    IRInst* emitCast(IRType* type, IRInst* value, bool fallbackToBuiltinCast = true);

    IRInst* emitVectorReshape(IRType* type, IRInst* value);

    IRInst* emitMakeUInt64(IRInst* low, IRInst* high);

    // Creates an RTTI object. Result is of `IRRTTIType`.
    IRInst* emitMakeRTTIObject(IRInst* typeInst);

    IRInst* emitMakeTargetTuple(IRType* type, UInt count, IRInst* const* args);

    IRInst* emitTargetTupleGetElement(
        IRType* elementType,
        IRInst* targetTupleVal,
        IRInst* indexVal);

    IRInst* emitMakeTuple(IRType* type, UInt count, IRInst* const* args);
    IRInst* emitMakeTuple(UInt count, IRInst* const* args);

    IRInst* emitMakeTuple(IRType* type, List<IRInst*> const& args)
    {
        if (args.getCount() == 1)
        {
            if (args[0]->getOp() == kIROp_Expand)
            {
                return args[0];
            }
        }
        return emitMakeTuple(type, args.getCount(), args.getBuffer());
    }

    IRInst* emitMakeTuple(List<IRInst*> const& args)
    {
        return emitMakeTuple(args.getCount(), args.getBuffer());
    }

    IRInst* emitMakeTuple(IRInst* arg0, IRInst* arg1)
    {
        IRInst* args[] = {arg0, arg1};
        return emitMakeTuple(SLANG_COUNT_OF(args), args);
    }

    IRInst* emitMakeValuePack(IRType* type, UInt count, IRInst* const* args);
    IRInst* emitMakeValuePack(UInt count, IRInst* const* args);

    IRInst* emitMakeWitnessPack(IRType* type, ArrayView<IRInst*> args)
    {
        return emitIntrinsicInst(
            type,
            kIROp_MakeWitnessPack,
            (UInt)args.getCount(),
            args.getBuffer());
    }

    IRInst* emitMakeString(IRInst* nativeStr);

    IRInst* emitGetNativeString(IRInst* str);

    IRInst* emitGetTupleElement(IRType* type, IRInst* tuple, int element)
    {
        return emitGetTupleElement(type, tuple, (UInt)element);
    }

    IRInst* emitGetTupleElement(IRType* type, IRInst* tuple, UInt element);
    IRInst* emitGetTupleElement(IRType* type, IRInst* tuple, IRInst* element);

    IRInst* emitCoopMatMapElementFunc(IRType* type, IRInst* tuple, IRInst* func);

    IRInst* emitGetElement(IRType* type, IRInst* arrayLikeType, IRIntegerValue element);
    IRInst* emitGetElementPtr(IRType* type, IRInst* arrayLikeType, IRIntegerValue element);

    IRInst* emitMakeResultError(IRType* resultType, IRInst* errorVal);
    IRInst* emitMakeResultValue(IRType* resultType, IRInst* val);
    IRInst* emitIsResultError(IRInst* result);
    IRInst* emitGetResultError(IRInst* result);
    IRInst* emitGetResultValue(IRInst* result);
    IRInst* emitOptionalHasValue(IRInst* optValue);
    IRInst* emitGetOptionalValue(IRInst* optValue);
    IRInst* emitMakeOptionalValue(IRInst* optType, IRInst* value);
    IRInst* emitMakeOptionalNone(IRInst* optType, IRInst* defaultValue);

    IRInst* emitDifferentialPairGetDifferential(IRType* diffType, IRInst* diffPair);
    IRInst* emitDifferentialValuePairGetDifferential(IRType* diffType, IRInst* diffPair);
    IRInst* emitDifferentialPtrPairGetDifferential(IRType* diffType, IRInst* diffPair);

    IRInst* emitDifferentialPairGetPrimal(IRInst* diffPair);
    IRInst* emitDifferentialValuePairGetPrimal(IRInst* diffPair);
    IRInst* emitDifferentialPtrPairGetPrimal(IRInst* diffPair);

    IRInst* emitDifferentialPairGetPrimal(IRType* primalType, IRInst* diffPair);
    IRInst* emitDifferentialValuePairGetPrimal(IRType* primalType, IRInst* diffPair);
    IRInst* emitDifferentialPtrPairGetPrimal(IRType* primalType, IRInst* diffPair);

    IRInst* emitDifferentialPairGetDifferentialUserCode(IRType* diffType, IRInst* diffPair);
    IRInst* emitDifferentialPairGetPrimalUserCode(IRInst* diffPair);
    IRInst* emitMakeVector(IRType* type, UInt argCount, IRInst* const* args);
    IRInst* emitMakeVectorFromScalar(IRType* type, IRInst* scalarValue);
    IRInst* emitMakeCompositeFromScalar(IRType* type, IRInst* scalarValue);

    IRInst* emitMakeVector(IRType* type, List<IRInst*> const& args)
    {
        return emitMakeVector(type, args.getCount(), args.getBuffer());
    }
    IRInst* emitMatrixReshape(IRType* type, IRInst* inst);

    IRInst* emitMakeMatrix(IRType* type, UInt argCount, IRInst* const* args);

    IRInst* emitMakeMatrixFromScalar(IRType* type, IRInst* scalarValue);

    IRInst* emitMakeCoopVector(IRType* type, UInt argCount, IRInst* const* args);

    IRInst* emitMakeArray(IRType* type, UInt argCount, IRInst* const* args);

    IRInst* emitMakeArrayList(IRType* type, UInt argCount, IRInst* const* args);

    IRInst* emitMakeArrayFromElement(IRType* type, IRInst* element);

    IRInst* emitMakeStruct(IRType* type, UInt argCount, IRInst* const* args);

    IRInst* emitMakeStruct(IRType* type, List<IRInst*> const& args)
    {
        return emitMakeStruct(type, args.getCount(), args.getBuffer());
    }

    IRInst* emitMakeTensorView(IRType* type, IRInst* val);

    IRInst* emitMakeExistential(IRType* type, IRInst* value, IRInst* witnessTable);

    IRInst* emitMakeExistentialWithRTTI(
        IRType* type,
        IRInst* value,
        IRInst* witnessTable,
        IRInst* rtti);

    IRInst* emitWrapExistential(
        IRType* type,
        IRInst* value,
        UInt slotArgCount,
        IRInst* const* slotArgs);

    IRInst* emitWrapExistential(
        IRType* type,
        IRInst* value,
        UInt slotArgCount,
        IRUse const* slotArgs)
    {
        List<IRInst*> slotArgVals;
        for (UInt ii = 0; ii < slotArgCount; ++ii)
            slotArgVals.add(slotArgs[ii].get());

        return emitWrapExistential(type, value, slotArgCount, slotArgVals.getBuffer());
    }

    IRInst* emitManagedPtrAttach(IRInst* managedPtrVar, IRInst* value);

    IRInst* emitManagedPtrDetach(IRType* type, IRInst* managedPtrVal);

    IRInst* emitGetNativePtr(IRInst* value);

    IRInst* emitGetManagedPtrWriteRef(IRInst* ptrToManagedPtr);

    IRInst* emitGpuForeach(List<IRInst*> args);

    IRUndefined* emitUndefined(IRType* type);

    IRInst* emitReinterpret(IRInst* type, IRInst* value);
    IRInst* emitOutImplicitCast(IRInst* type, IRInst* value);
    IRInst* emitInOutImplicitCast(IRInst* type, IRInst* value);

    IRInst* emitByteAddressBufferStore(IRInst* byteAddressBuffer, IRInst* offset, IRInst* value);
    IRInst* emitByteAddressBufferStore(
        IRInst* byteAddressBuffer,
        IRInst* offset,
        IRInst* alignment,
        IRInst* value);

    IRInst* emitEmbeddedDownstreamIR(CodeGenTarget target, ISlangBlob* blob);

    IRFunc* createFunc();
    IRGlobalVar* createGlobalVar(IRType* valueType);
    IRGlobalVar* createGlobalVar(IRType* valueType, AddressSpace addressSpace);
    IRGlobalParam* createGlobalParam(IRType* valueType);

    /// Creates an IRWitnessTable value.
    /// @param baseType: The comformant-to type of this witness.
    /// @param subType: The type that is doing the conforming.
    IRWitnessTable* createWitnessTable(IRType* baseType, IRType* subType);
    IRWitnessTableEntry* createWitnessTableEntry(
        IRWitnessTable* witnessTable,
        IRInst* requirementKey,
        IRInst* satisfyingVal);

    IRInst* createThisTypeWitness(IRType* interfaceType);

    IRInst* getTypeEqualityWitness(IRType* witnessType, IRType* type1, IRType* type2);

    IRInterfaceRequirementEntry* createInterfaceRequirementEntry(
        IRInst* requirementKey,
        IRInst* requirementVal);

    // Create an initially empty `struct` type.
    IRStructType* createStructType();

    // Create an initially empty `class` type.
    IRClassType* createClassType();

    // Create an an `enum` type with the given tag type.
    IREnumType* createEnumType(IRType* tagType);

    // Create an initially empty `GLSLShaderStorageBufferType` type.
    IRGLSLShaderStorageBufferType* createGLSLShaderStorableBufferType();
    IRGLSLShaderStorageBufferType* createGLSLShaderStorableBufferType(
        UInt operandCount,
        IRInst* const* operands);

    // Create an empty `interface` type.
    IRInterfaceType* createInterfaceType(UInt operandCount, IRInst* const* operands);

    // Create a global "key" to use for indexing into a `struct` type.
    IRStructKey* createStructKey();

    // Create a field nested in a struct type, declaring that
    // the specified field key maps to a field with the specified type.
    IRStructField* createStructField(IRType* aggType, IRStructKey* fieldKey, IRType* fieldType);

    IRGeneric* createGeneric();
    IRGeneric* emitGeneric();

    // Low-level operation for creating a type.
    IRType* getType(IROp op, UInt operandCount, IRInst* const* operands);
    IRType* getType(IROp op);
    IRType* getType(IROp op, IRInst* operand0);

    /// Create an empty basic block.
    ///
    /// The created block will not be inserted into the current
    /// function; call `insertBlock()` to attach the block
    /// at an appropriate point.
    ///
    IRBlock* createBlock();

    /// Insert a block into the current function.
    ///
    /// This attaches the given `block` to the current function,
    /// and makes it the current block for
    /// new instructions that get emitted.
    ///
    void insertBlock(IRBlock* block);

    /// Emit a new block into the current function.
    ///
    /// This function is equivalent to using `createBlock()`
    /// and then `insertBlock()`.
    ///
    IRBlock* emitBlock();

    static void insertBlockAlongEdge(IRModule* module, IREdge const& edge);

    IRParam* createParam(IRType* type);
    IRParam* emitParam(IRType* type);
    IRParam* emitParamAtHead(IRType* type);

    IRInst* emitAllocObj(IRType* type);

    IRVar* emitVar(IRType* type);
    IRVar* emitVar(IRType* type, AddressSpace addressSpace);

    IRInst* emitLoad(IRType* type, IRInst* ptr);
    IRInst* emitLoad(IRType* type, IRInst* ptr, IRInst* align);
    IRInst* emitLoad(IRType* type, IRInst* ptr, IRAlignedAttr* align);
    IRInst* emitLoad(IRInst* ptr);

    IRInst* emitLoadReverseGradient(IRType* type, IRInst* diffValue);
    IRInst* emitReverseGradientDiffPairRef(IRType* type, IRInst* primalVar, IRInst* diffVar);
    IRInst* emitPrimalParamRef(IRInst* param);
    IRInst* emitDiffParamRef(IRType* type, IRInst* param);

    IRInst* emitStore(IRInst* dstPtr, IRInst* srcVal);
    IRInst* emitStore(IRInst* dstPtr, IRInst* srcVal, IRInst* align);

    IRInst* emitAtomicStore(IRInst* dstPtr, IRInst* srcVal, IRInst* memoryOrder);

    IRInst* emitImageLoad(IRType* type, ShortList<IRInst*> params);

    IRInst* emitImageStore(IRType* type, ShortList<IRInst*> params);

    IRInst* emitIsType(IRInst* value, IRInst* witness, IRInst* typeOperand, IRInst* targetWitness);

    IRInst* emitFieldExtract(IRInst* base, IRInst* fieldKey);

    IRInst* emitFieldExtract(IRType* type, IRInst* base, IRInst* field);

    IRInst* emitFieldAddress(IRInst* basePtr, IRInst* fieldKey);

    IRInst* emitFieldAddress(IRType* type, IRInst* basePtr, IRInst* field);

    IRInst* emitElementExtract(IRType* type, IRInst* base, IRInst* index);

    IRInst* emitElementExtract(IRInst* base, IRInst* index);

    IRInst* emitElementExtract(IRInst* base, IRIntegerValue index);

    IRInst* emitElementExtract(IRInst* base, const ArrayView<IRInst*>& accessChain);

    IRInst* emitElementAddress(IRType* type, IRInst* basePtr, IRInst* index);

    IRInst* emitElementAddress(IRInst* basePtr, IRInst* index);

    IRInst* emitElementAddress(IRInst* basePtr, IRIntegerValue index);

    IRInst* emitElementAddress(IRInst* basePtr, const ArrayView<IRInst*>& accessChain);
    IRInst* emitElementAddress(
        IRInst* basePtr,
        const ArrayView<IRInst*>& accessChain,
        const ArrayView<IRInst*>& types);

    IRInst* emitUpdateElement(IRInst* base, IRInst* index, IRInst* newElement);
    IRInst* emitUpdateElement(IRInst* base, IRIntegerValue index, IRInst* newElement);
    IRInst* emitUpdateElement(IRInst* base, ArrayView<IRInst*> accessChain, IRInst* newElement);
    IRInst* emitGetOffsetPtr(IRInst* base, IRInst* offset);
    IRInst* emitGetAddress(IRType* type, IRInst* value);

    IRInst* emitSwizzle(
        IRType* type,
        IRInst* base,
        UInt elementCount,
        IRInst* const* elementIndices);

    IRInst* emitSwizzle(
        IRType* type,
        IRInst* base,
        UInt elementCount,
        uint64_t const* elementIndices);

    IRInst* emitSwizzle(
        IRType* type,
        IRInst* base,
        UInt elementCount,
        uint32_t const* elementIndices);

    IRInst* emitSwizzleSet(
        IRType* type,
        IRInst* base,
        IRInst* source,
        UInt elementCount,
        IRInst* const* elementIndices);

    IRInst* emitSwizzleSet(
        IRType* type,
        IRInst* base,
        IRInst* source,
        UInt elementCount,
        uint64_t const* elementIndices);

    IRInst* emitSwizzleSet(
        IRType* type,
        IRInst* base,
        IRInst* source,
        UInt elementCount,
        uint32_t const* elementIndices);

    IRInst* emitSwizzledStore(
        IRInst* dest,
        IRInst* source,
        UInt elementCount,
        IRInst* const* elementIndices);

    IRInst* emitSwizzledStore(
        IRInst* dest,
        IRInst* source,
        UInt elementCount,
        uint32_t const* elementIndices);

    IRInst* emitSwizzledStore(
        IRInst* dest,
        IRInst* source,
        UInt elementCount,
        uint64_t const* elementIndices);


    IRInst* emitReturn(IRInst* val);

    IRInst* emitYield(IRInst* val);

    IRInst* emitReturn();

    IRInst* emitThrow(IRInst* val);

    IRInst* emitDefer(IRBlock* deferBlock, IRBlock* mergeBlock, IRBlock* scopeEndBlock);

    IRInst* emitDiscard();

    IRInst* emitCheckpointObject(IRInst* value);
    IRInst* emitLoopExitValue(IRInst* value);

    IRInst* emitUnreachable();
    IRInst* emitMissingReturn();

    IRInst* emitBranch(IRBlock* block);

    IRInst* emitBranch(IRBlock* block, Int argCount, IRInst* const* args);

    IRInst* emitBreak(IRBlock* target);

    IRInst* emitContinue(IRBlock* target);

    IRInst* emitLoop(IRBlock* target, IRBlock* breakBlock, IRBlock* continueBlock);

    IRInst* emitLoop(
        IRBlock* target,
        IRBlock* breakBlock,
        IRBlock* continueBlock,
        Int argCount,
        IRInst* const* args);

    IRInst* emitBranch(IRInst* val, IRBlock* trueBlock, IRBlock* falseBlock);

    IRInst* emitIf(IRInst* val, IRBlock* trueBlock, IRBlock* afterBlock);

    IRIfElse* emitIfElse(IRInst* val, IRBlock* trueBlock, IRBlock* falseBlock, IRBlock* afterBlock);

    // Create basic blocks and insert an `IfElse` inst at current position that jumps into the
    // blocks. The current insert position is changed to inside `outTrueBlock` after the call.
    IRInst* emitIfElseWithBlocks(
        IRInst* val,
        IRBlock*& outTrueBlock,
        IRBlock*& outFalseBlock,
        IRBlock*& outAfterBlock);

    // Create basic blocks and insert an `If` inst at current position that jumps into the blocks.
    // The current insert position is changed to inside `outTrueBlock` after the call.
    IRInst* emitIfWithBlocks(IRInst* val, IRBlock*& outTrueBlock, IRBlock*& outAfterBlock);

    IRInst* emitLoopTest(IRInst* val, IRBlock* bodyBlock, IRBlock* breakBlock);

    IRInst* emitSwitch(
        IRInst* val,
        IRBlock* breakLabel,
        IRBlock* defaultLabel,
        UInt caseArgCount,
        IRInst* const* caseArgs);

    IRInst* emitBeginFragmentShaderInterlock()
    {
        return emitIntrinsicInst(getVoidType(), kIROp_BeginFragmentShaderInterlock, 0, nullptr);
    }

    IRInst* emitEndFragmentShaderInterlock()
    {
        return emitIntrinsicInst(getVoidType(), kIROp_EndFragmentShaderInterlock, 0, nullptr);
    }

    IRGlobalGenericParam* emitGlobalGenericParam(IRType* type);

    IRGlobalGenericParam* emitGlobalGenericTypeParam()
    {
        return emitGlobalGenericParam(getTypeKind());
    }

    IRGlobalGenericParam* emitGlobalGenericWitnessTableParam(IRType* comformanceType)
    {
        return emitGlobalGenericParam(getWitnessTableType(comformanceType));
    }

    IRBindGlobalGenericParam* emitBindGlobalGenericParam(IRInst* param, IRInst* val);

    IRDecoration* addBindExistentialSlotsDecoration(
        IRInst* value,
        UInt argCount,
        IRInst* const* args);

    IRInst* emitExtractTaggedUnionTag(IRInst* val);

    IRInst* emitExtractTaggedUnionPayload(IRType* type, IRInst* val, IRInst* tag);

    IRInst* emitBitCast(IRType* type, IRInst* val);

    IRInst* emitSizeOf(IRInst* sizedType);

    IRInst* emitAlignOf(IRInst* sizedType);

    IRInst* emitCountOf(IRType* type, IRInst* sizedType);

    IRInst* emitCastPtrToBool(IRInst* val);
    IRInst* emitCastPtrToInt(IRInst* val);
    IRInst* emitCastIntToPtr(IRType* ptrType, IRInst* val);

    IRGlobalConstant* emitGlobalConstant(IRType* type);

    IRGlobalConstant* emitGlobalConstant(IRType* type, IRInst* val);

    IRInst* emitWaveMaskBallot(IRType* type, IRInst* mask, IRInst* condition);
    IRInst* emitWaveMaskMatch(IRType* type, IRInst* mask, IRInst* value);

    IRInst* emitBitAnd(IRType* type, IRInst* left, IRInst* right);
    IRInst* emitBitOr(IRType* type, IRInst* left, IRInst* right);
    IRInst* emitBitNot(IRType* type, IRInst* value);
    IRInst* emitNeg(IRType* type, IRInst* value);
    IRInst* emitNot(IRType* type, IRInst* value);

    IRInst* emitAdd(IRType* type, IRInst* left, IRInst* right);
    IRInst* emitSub(IRType* type, IRInst* left, IRInst* right);
    IRInst* emitMul(IRType* type, IRInst* left, IRInst* right);
    IRInst* emitDiv(IRType* type, IRInst* numerator, IRInst* denominator);
    IRInst* emitEql(IRInst* left, IRInst* right);
    IRInst* emitNeq(IRInst* left, IRInst* right);
    IRInst* emitLess(IRInst* left, IRInst* right);
    IRInst* emitGeq(IRInst* left, IRInst* right);

    IRInst* emitShr(IRType* type, IRInst* op0, IRInst* op1);
    IRInst* emitShl(IRType* type, IRInst* op0, IRInst* op1);

    IRInst* emitAnd(IRType* type, IRInst* left, IRInst* right);
    IRInst* emitOr(IRType* type, IRInst* left, IRInst* right);

    IRSPIRVAsmOperand* emitSPIRVAsmOperandLiteral(IRInst* literal);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandInst(IRInst* inst);
    IRSPIRVAsmOperand* createSPIRVAsmOperandInst(IRInst* inst);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandConvertTexel(IRInst* inst);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandRayPayloadFromLocation(IRInst* inst);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandRayAttributeFromLocation(IRInst* inst);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandRayCallableFromLocation(IRInst* inst);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandId(IRInst* inst);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandResult();
    IRSPIRVAsmOperand* emitSPIRVAsmOperandEnum(IRInst* inst);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandEnum(IRInst* inst, IRType* constantType);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandBuiltinVar(IRInst* type, IRInst* builtinKind);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandGLSL450Set();
    IRSPIRVAsmOperand* emitSPIRVAsmOperandDebugPrintfSet();
    IRSPIRVAsmOperand* emitSPIRVAsmOperandSampledType(IRType* elementType);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandImageType(IRInst* element);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandSampledImageType(IRInst* element);
    IRSPIRVAsmOperand* emitSPIRVAsmOperandTruncate();
    IRSPIRVAsmOperand* emitSPIRVAsmOperandEntryPoint();
    IRSPIRVAsmInst* emitSPIRVAsmInst(IRInst* opcode, List<IRInst*> operands);
    IRSPIRVAsm* emitSPIRVAsm(IRType* type);
    IRInst* emitGenericAsm(UnownedStringSlice asmText);

    IRInst* emitRWStructuredBufferGetElementPtr(IRInst* structuredBuffer, IRInst* index);

    IRInst* emitNonUniformResourceIndexInst(IRInst* val);

    IRMetalSetVertex* emitMetalSetVertex(IRInst* index, IRInst* vertex);
    IRMetalSetPrimitive* emitMetalSetPrimitive(IRInst* index, IRInst* primitive);
    IRMetalSetIndices* emitMetalSetIndices(IRInst* index, IRInst* indices);

    //
    // Decorations
    //

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* const* operands, Int operandCount);

    IRDecoration* addDecoration(IRInst* value, IROp op)
    {
        return addDecoration(value, op, (IRInst* const*)nullptr, 0);
    }

    IRDecoration* addDecorationIfNotExist(IRInst* value, IROp op)
    {
        if (auto decor = value->findDecorationImpl(op))
            return decor;
        return addDecoration(value, op);
    }

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* operand)
    {
        return addDecoration(value, op, &operand, 1);
    }

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* operand0, IRInst* operand1)
    {
        IRInst* operands[] = {operand0, operand1};
        return addDecoration(value, op, operands, SLANG_COUNT_OF(operands));
    }

    IRDecoration* addDecoration(
        IRInst* value,
        IROp op,
        IRInst* operand0,
        IRInst* operand1,
        IRInst* operand2)
    {
        IRInst* operands[] = {operand0, operand1, operand2};
        return addDecoration(value, op, operands, SLANG_COUNT_OF(operands));
    }

    IRDecoration* addDecoration(
        IRInst* value,
        IROp op,
        IRInst* operand0,
        IRInst* operand1,
        IRInst* operand2,
        IRInst* operand3)
    {
        IRInst* operands[] = {operand0, operand1, operand2, operand3};
        return addDecoration(value, op, operands, SLANG_COUNT_OF(operands));
    }

    template<typename T>
    IRDecoration* addSimpleDecoration(IRInst* value)
    {
        return addDecoration(value, IROp(T::kOp), (IRInst* const*)nullptr, 0);
    }

    void addHighLevelDeclDecoration(IRInst* value, Decl* decl);

    IRDecoration* addResultWitnessDecoration(IRInst* value, IRInst* witness)
    {
        return addDecoration(value, kIROp_ResultWitnessDecoration, witness);
    }

    IRDecoration* addTargetSystemValueDecoration(
        IRInst* value,
        UnownedStringSlice sysValName,
        UInt index = 0)
    {
        IRInst* operands[] = {getStringValue(sysValName), getIntValue(getIntType(), index)};
        return addDecoration(
            value,
            kIROp_TargetSystemValueDecoration,
            operands,
            SLANG_COUNT_OF(operands));
    }

    //    void addLayoutDecoration(IRInst* value, Layout* layout);
    IRLayoutDecoration* addLayoutDecoration(IRInst* value, IRLayout* layout);

    IRDecoration* addTargetBuiltinVarDecoration(
        IRInst* value,
        IRTargetBuiltinVarName builtinVarName)
    {
        IRInst* operands[] = {getIntValue((IRIntegerValue)builtinVarName)};
        return addDecoration(
            value,
            kIROp_TargetBuiltinVarDecoration,
            operands,
            SLANG_COUNT_OF(operands));
    }

    //    IRLayout* getLayout(Layout* astLayout);

    IRTypeSizeAttr* getTypeSizeAttr(LayoutResourceKind kind, LayoutSize size);
    IRVarOffsetAttr* getVarOffsetAttr(LayoutResourceKind kind, UInt offset, UInt space = 0);
    IRPendingLayoutAttr* getPendingLayoutAttr(IRLayout* pendingLayout);
    IRStructFieldLayoutAttr* getFieldLayoutAttr(IRInst* key, IRVarLayout* layout);
    IRTupleFieldLayoutAttr* getTupleFieldLayoutAttr(IRTypeLayout* layout);
    IRCaseTypeLayoutAttr* getCaseTypeLayoutAttr(IRTypeLayout* layout);

    IRSemanticAttr* getSemanticAttr(IROp op, String const& name, UInt index);
    IRSystemValueSemanticAttr* getSystemValueSemanticAttr(String const& name, UInt index)
    {
        return cast<IRSystemValueSemanticAttr>(
            getSemanticAttr(kIROp_SystemValueSemanticAttr, name, index));
    }
    IRUserSemanticAttr* getUserSemanticAttr(String const& name, UInt index)
    {
        return cast<IRUserSemanticAttr>(getSemanticAttr(kIROp_UserSemanticAttr, name, index));
    }

    IRStageAttr* getStageAttr(Stage stage);

    IRAttr* getAttr(IROp op, UInt operandCount, IRInst* const* operands);

    IRAttr* getAttr(IROp op, List<IRInst*> const& operands)
    {
        return getAttr(op, operands.getCount(), operands.getBuffer());
    }

    IRAttr* getAttr(IROp op) { return getAttr(op, 0, nullptr); }

    IRTypeLayout* getTypeLayout(IROp op, List<IRInst*> const& operands);
    IRVarLayout* getVarLayout(List<IRInst*> const& operands);
    IREntryPointLayout* getEntryPointLayout(IRVarLayout* paramsLayout, IRVarLayout* resultLayout);

    void addPhysicalTypeDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_PhysicalTypeDecoration);
    }

    void addAlignedAddressDecoration(IRInst* value, IRInst* alignment)
    {
        addDecoration(value, kIROp_AlignedAddressDecoration, alignment);
    }

    void addNameHintDecoration(IRInst* value, IRStringLit* name)
    {
        addDecoration(value, kIROp_NameHintDecoration, name);
    }

    void addNameHintDecoration(IRInst* value, UnownedStringSlice const& text)
    {
        addNameHintDecoration(value, getStringValue(text));
    }

    void addUserTypeNameDecoration(IRInst* value, IRStringLit* name)
    {
        addDecoration(value, kIROp_UserTypeNameDecoration, name);
    }

    void addBinaryInterfaceTypeDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_BinaryInterfaceTypeDecoration);
    }

    void addGLSLOuterArrayDecoration(IRInst* value, UnownedStringSlice const& text)
    {
        addDecoration(value, kIROp_GLSLOuterArrayDecoration, getStringValue(text));
    }

    void addGLPositionOutputDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_GLPositionOutputDecoration);
    }

    void addGLPositionInputDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_GLPositionInputDecoration);
    }

    void addInterpolationModeDecoration(IRInst* value, IRInterpolationMode mode)
    {
        addDecoration(
            value,
            kIROp_InterpolationModeDecoration,
            getIntValue(getIntType(), IRIntegerValue(mode)));
    }

    void addLoopControlDecoration(IRInst* value, IRLoopControl mode)
    {
        addDecoration(
            value,
            kIROp_LoopControlDecoration,
            getIntValue(getIntType(), IRIntegerValue(mode)));
    }

    void addLoopMaxItersDecoration(IRInst* value, IRIntegerValue iters)
    {
        addDecoration(value, kIROp_LoopMaxItersDecoration, getIntValue(iters));
    }

    void addLoopMaxItersDecoration(IRInst* value, IRInst* iters)
    {
        addDecoration(value, kIROp_LoopMaxItersDecoration, iters);
    }

    void addLoopForceUnrollDecoration(IRInst* value, IntegerLiteralValue iters)
    {
        addDecoration(value, kIROp_ForceUnrollDecoration, getIntValue(getIntType(), iters));
    }

    IRSemanticDecoration* addSemanticDecoration(
        IRInst* value,
        UnownedStringSlice const& text,
        IRIntegerValue index = 0)
    {
        return as<IRSemanticDecoration>(addDecoration(
            value,
            kIROp_SemanticDecoration,
            getStringValue(text),
            getIntValue(getIntType(), index)));
    }

    void addConstructorDecoration(IRInst* value, bool synthesizedConstructor)
    {
        addDecoration(value, kIROp_ConstructorDecoration, getBoolValue(synthesizedConstructor));
    }

    void addRequireSPIRVDescriptorIndexingExtensionDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_RequireSPIRVDescriptorIndexingExtensionDecoration);
    }

    void addTargetIntrinsicDecoration(
        IRInst* value,
        IRInst* caps,
        UnownedStringSlice const& definition,
        UnownedStringSlice const& predicate,
        IRInst* typeScrutinee)
    {
        typeScrutinee ? addDecoration(
                            value,
                            kIROp_TargetIntrinsicDecoration,
                            caps,
                            getStringValue(definition),
                            getStringValue(predicate),
                            typeScrutinee)
                      : addDecoration(
                            value,
                            kIROp_TargetIntrinsicDecoration,
                            caps,
                            getStringValue(definition));
    }

    void addTargetIntrinsicDecoration(
        IRInst* value,
        CapabilitySet const& caps,
        UnownedStringSlice const& definition,
        UnownedStringSlice const& predicate = UnownedStringSlice{},
        IRInst* typeScrutinee = nullptr)
    {
        addTargetIntrinsicDecoration(
            value,
            getCapabilityValue(caps),
            definition,
            predicate,
            typeScrutinee);
    }

    void addTargetDecoration(IRInst* value, IRInst* caps)
    {
        addDecoration(value, kIROp_TargetDecoration, caps);
    }

    void addTargetDecoration(IRInst* value, CapabilitySet const& caps)
    {
        addTargetDecoration(value, getCapabilityValue(caps));
    }

    void addRequireGLSLExtensionDecoration(IRInst* value, UnownedStringSlice const& extensionName)
    {
        addDecoration(value, kIROp_RequireGLSLExtensionDecoration, getStringValue(extensionName));
    }

    void addRequireGLSLVersionDecoration(IRInst* value, Int version)
    {
        addDecoration(
            value,
            kIROp_RequireGLSLVersionDecoration,
            getIntValue(getIntType(), IRIntegerValue(version)));
    }

    void addRequireWGSLExtensionDecoration(IRInst* value, UnownedStringSlice const& extensionName)
    {
        addDecoration(value, kIROp_RequireWGSLExtensionDecoration, getStringValue(extensionName));
    }

    void addRequirePreludeDecoration(
        IRInst* value,
        const CapabilitySet& caps,
        UnownedStringSlice prelude)
    {
        addDecoration(
            value,
            kIROp_RequirePreludeDecoration,
            getCapabilityValue(caps),
            getStringValue(prelude));
    }

    IRInst* getSemanticVersionValue(SemanticVersion const& value)
    {
        SemanticVersion::RawValue rawValue = value.getRawValue();
        return getIntValue(getBasicType(BaseType::UInt64), rawValue);
    }

    void addRequireSPIRVVersionDecoration(IRInst* value, const SemanticVersion& version)
    {
        addDecoration(value, kIROp_RequireSPIRVVersionDecoration, getSemanticVersionValue(version));
    }

    void addSPIRVNonUniformResourceDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_SPIRVNonUniformResourceDecoration);
    }

    void addRequireCUDASMVersionDecoration(IRInst* value, const SemanticVersion& version)
    {
        addDecoration(
            value,
            kIROp_RequireCUDASMVersionDecoration,
            getSemanticVersionValue(version));
    }

    void addRequireCapabilityAtomDecoration(IRInst* value, CapabilityName atom)
    {
        addDecoration(
            value,
            kIROp_RequireCapabilityAtomDecoration,
            getIntValue(getUIntType(), IRIntegerValue(atom)));
    }

    void addPatchConstantFuncDecoration(IRInst* value, IRInst* patchConstantFunc)
    {
        addDecoration(value, kIROp_PatchConstantFuncDecoration, patchConstantFunc);
    }

    void addImportDecoration(IRInst* value, UnownedStringSlice const& mangledName)
    {
        addDecoration(value, kIROp_ImportDecoration, getStringValue(mangledName));
    }

    void addExportDecoration(IRInst* value, UnownedStringSlice const& mangledName)
    {
        addDecoration(value, kIROp_ExportDecoration, getStringValue(mangledName));
    }

    void addUserExternDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_UserExternDecoration);
    }

    void addExternCppDecoration(IRInst* value, UnownedStringSlice const& mangledName)
    {
        addDecoration(value, kIROp_ExternCppDecoration, getStringValue(mangledName));
    }

    void addExternCDecoration(IRInst* value) { addDecoration(value, kIROp_ExternCDecoration); }

    void addDebugLocationDecoration(
        IRInst* value,
        IRInst* debugSource,
        IRIntegerValue line,
        IRIntegerValue col)
    {
        addDecoration(
            value,
            kIROp_DebugLocationDecoration,
            debugSource,
            getIntValue(getUIntType(), line),
            getIntValue(getUIntType(), col));
    }

    void addDebugFunctionDecoration(IRInst* value, IRInst* debugFunction)
    {
        addDecoration(value, kIROp_DebugFuncDecoration, debugFunction);
    }

    void addUnsafeForceInlineDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_UnsafeForceInlineEarlyDecoration);
    }

    void addForceInlineDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_ForceInlineDecoration);
    }

    void addAutoDiffOriginalValueDecoration(IRInst* value, IRInst* originalVal)
    {
        addDecoration(value, kIROp_AutoDiffOriginalValueDecoration, originalVal);
    }

    void addForwardDifferentiableDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_ForwardDifferentiableDecoration);
    }

    void addBackwardDifferentiableDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_BackwardDifferentiableDecoration);
    }

    void addForwardDerivativeDecoration(IRInst* value, IRInst* fwdFunc)
    {
        addDecoration(value, kIROp_ForwardDerivativeDecoration, fwdFunc);
    }

    void addUserDefinedBackwardDerivativeDecoration(IRInst* value, IRInst* fwdFunc)
    {
        addDecoration(value, kIROp_UserDefinedBackwardDerivativeDecoration, fwdFunc);
    }

    void addBackwardDerivativePrimalDecoration(IRInst* value, IRInst* jvpFn)
    {
        addDecoration(value, kIROp_BackwardDerivativePrimalDecoration, jvpFn);
    }

    void addBackwardDerivativePrimalReturnDecoration(IRInst* value, IRInst* retVal)
    {
        addDecoration(value, kIROp_BackwardDerivativePrimalReturnDecoration, retVal);
    }

    void addBackwardDerivativePropagateDecoration(IRInst* value, IRInst* jvpFn)
    {
        addDecoration(value, kIROp_BackwardDerivativePropagateDecoration, jvpFn);
    }

    void addBackwardDerivativeDecoration(IRInst* value, IRInst* jvpFn)
    {
        addDecoration(value, kIROp_BackwardDerivativeDecoration, jvpFn);
    }

    void addBackwardDerivativeIntermediateTypeDecoration(IRInst* value, IRInst* jvpFn)
    {
        addDecoration(value, kIROp_BackwardDerivativeIntermediateTypeDecoration, jvpFn);
    }

    void addBackwardDerivativePrimalContextDecoration(IRInst* value, IRInst* ctx)
    {
        addDecoration(value, kIROp_BackwardDerivativePrimalContextDecoration, ctx);
    }

    void addPrimalSubstituteDecoration(IRInst* value, IRInst* jvpFn)
    {
        addDecoration(value, kIROp_PrimalSubstituteDecoration, jvpFn);
    }

    void addLoopCounterDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_LoopCounterDecoration);
    }

    void addLoopExitPrimalValueDecoration(IRInst* value, IRInst* primalInst, IRInst* exitValue)
    {
        addDecoration(value, kIROp_LoopExitPrimalValueDecoration, primalInst, exitValue);
    }

    void addLoopCounterUpdateDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_LoopCounterUpdateDecoration);
    }

    void markInstAsPrimal(IRInst* value) { addDecoration(value, kIROp_PrimalInstDecoration); }

    void markInstAsDifferential(IRInst* value)
    {
        addDecoration(value, kIROp_DifferentialInstDecoration, nullptr);
    }

    void markInstAsMixedDifferential(IRInst* value)
    {
        addDecoration(value, kIROp_MixedDifferentialInstDecoration, nullptr);
    }

    void markInstAsMixedDifferential(IRInst* value, IRType* pairType)
    {
        addDecoration(value, kIROp_MixedDifferentialInstDecoration, pairType);
    }

    void markInstAsDifferential(IRInst* value, IRType* primalType)
    {
        addDecoration(value, kIROp_DifferentialInstDecoration, primalType);
    }

    void markInstAsDifferential(IRInst* value, IRType* primalType, IRInst* primalInst)
    {
        addDecoration(value, kIROp_DifferentialInstDecoration, primalType, primalInst);
    }

    void markInstAsDifferential(
        IRInst* value,
        IRType* primalType,
        IRInst* primalInst,
        IRInst* witnessTable)
    {
        IRInst* args[] = {primalType, primalInst, witnessTable};
        addDecoration(value, kIROp_DifferentialInstDecoration, args, 3);
    }

    void addCOMWitnessDecoration(IRInst* value, IRInst* witnessTable)
    {
        addDecoration(value, kIROp_COMWitnessDecoration, &witnessTable, 1);
    }

    void addDllImportDecoration(
        IRInst* value,
        UnownedStringSlice const& libraryName,
        UnownedStringSlice const& functionName)
    {
        addDecoration(
            value,
            kIROp_DllImportDecoration,
            getStringValue(libraryName),
            getStringValue(functionName));
    }

    void addDllExportDecoration(IRInst* value, UnownedStringSlice const& functionName)
    {
        addDecoration(value, kIROp_DllExportDecoration, getStringValue(functionName));
    }

    void addTorchEntryPointDecoration(IRInst* value, UnownedStringSlice const& functionName)
    {
        addDecoration(value, kIROp_TorchEntryPointDecoration, getStringValue(functionName));
    }

    void addAutoPyBindCudaDecoration(IRInst* value, UnownedStringSlice const& functionName)
    {
        addDecoration(value, kIROp_AutoPyBindCudaDecoration, getStringValue(functionName));
    }

    void addPyExportDecoration(IRInst* value, UnownedStringSlice const& exportName)
    {
        addDecoration(value, kIROp_PyExportDecoration, getStringValue(exportName));
    }

    void addCudaDeviceExportDecoration(IRInst* value, UnownedStringSlice const& functionName)
    {
        addDecoration(value, kIROp_CudaDeviceExportDecoration, getStringValue(functionName));
    }

    void addCudaHostDecoration(IRInst* value) { addDecoration(value, kIROp_CudaHostDecoration); }

    void addCudaKernelDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_CudaKernelDecoration);
    }

    void addCudaKernelForwardDerivativeDecoration(IRInst* value, IRInst* func)
    {
        addDecoration(value, kIROp_CudaKernelForwardDerivativeDecoration, func);
    }

    void addCudaKernelBackwardDerivativeDecoration(IRInst* value, IRInst* func)
    {
        addDecoration(value, kIROp_CudaKernelBackwardDerivativeDecoration, func);
    }

    void addAutoPyBindExportInfoDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_AutoPyBindExportInfoDecoration);
    }

    void addEntryPointDecoration(
        IRInst* value,
        Profile profile,
        UnownedStringSlice const& name,
        UnownedStringSlice const& moduleName)
    {
        IRInst* operands[] = {
            getIntValue(getIntType(), profile.raw),
            getStringValue(name),
            getStringValue(moduleName)};
        addDecoration(value, kIROp_EntryPointDecoration, operands, SLANG_COUNT_OF(operands));
    }

    void addKeepAliveDecoration(IRInst* value) { addDecoration(value, kIROp_KeepAliveDecoration); }

    void addPublicDecoration(IRInst* value) { addDecoration(value, kIROp_PublicDecoration); }
    void addHLSLExportDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_HLSLExportDecoration);
    }
    void addHasExplicitHLSLBindingDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_HasExplicitHLSLBindingDecoration);
    }
    void addDefaultValueDecoration(IRInst* value, IRInst* defaultValue)
    {
        addDecoration(value, kIROp_DefaultValueDecoration, defaultValue);
    }
    void addNVAPIMagicDecoration(IRInst* value, UnownedStringSlice const& name)
    {
        addDecoration(value, kIROp_NVAPIMagicDecoration, getStringValue(name));
    }

    void addNVAPISlotDecoration(
        IRInst* value,
        UnownedStringSlice const& registerName,
        UnownedStringSlice const& spaceName)
    {
        addDecoration(
            value,
            kIROp_NVAPISlotDecoration,
            getStringValue(registerName),
            getStringValue(spaceName));
    }

    void addNonCopyableTypeDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_NonCopyableTypeDecoration);
    }

    void addDynamicUniformDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_DynamicUniformDecoration);
    }

    void addAutoDiffBuiltinDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_AutoDiffBuiltinDecoration);
    }

    /// Add a decoration that indicates that the given `inst` depends on the given `dependency`.
    ///
    /// This decoration can be used to ensure that a value that an instruction
    /// implicitly depends on cannot be eliminated so long as the instruction
    /// itself is kept alive.
    ///
    void addDependsOnDecoration(IRInst* inst, IRInst* dependency)
    {
        addDecoration(inst, kIROp_DependsOnDecoration, dependency);
    }

    void addFormatDecoration(IRInst* inst, ImageFormat format)
    {
        addFormatDecoration(inst, getIntValue(getIntType(), IRIntegerValue(format)));
    }

    void addFormatDecoration(IRInst* inst, IRInst* format)
    {
        addDecoration(inst, kIROp_FormatDecoration, format);
    }

    void addRTTITypeSizeDecoration(IRInst* inst, IRIntegerValue value)
    {
        addDecoration(inst, kIROp_RTTITypeSizeDecoration, getIntValue(getIntType(), value));
    }

    void addAnyValueSizeDecoration(IRInst* inst, IRIntegerValue value)
    {
        addDecoration(inst, kIROp_AnyValueSizeDecoration, getIntValue(getIntType(), value));
    }

    void addDispatchFuncDecoration(IRInst* inst, IRInst* func)
    {
        addDecoration(inst, kIROp_DispatchFuncDecoration, func);
    }

    void addStaticRequirementDecoration(IRInst* inst)
    {
        addDecoration(inst, kIROp_StaticRequirementDecoration);
    }

    void addSpecializeDecoration(IRInst* inst) { addDecoration(inst, kIROp_SpecializeDecoration); }

    void addComInterfaceDecoration(IRInst* inst, UnownedStringSlice guid)
    {
        addDecoration(inst, kIROp_ComInterfaceDecoration, getStringValue(guid));
    }

    void addTypeConstraintDecoration(IRInst* inst, IRInst* constraintType)
    {
        addDecoration(inst, kIROp_TypeConstraintDecoration, constraintType);
    }

    void addBuiltinDecoration(IRInst* inst) { addDecoration(inst, kIROp_BuiltinDecoration); }

    void addSequentialIDDecoration(IRInst* inst, IRIntegerValue id)
    {
        addDecoration(inst, kIROp_SequentialIDDecoration, getIntValue(getUIntType(), id));
    }

    void addDynamicDispatchWitnessDecoration(IRInst* inst)
    {
        addDecoration(inst, kIROp_DynamicDispatchWitnessDecoration);
    }

    void addVulkanRayPayloadDecoration(IRInst* inst, int location)
    {
        addDecoration(inst, kIROp_VulkanRayPayloadDecoration, getIntValue(getIntType(), location));
    }

    void addVulkanRayPayloadInDecoration(IRInst* inst, int location)
    {
        addDecoration(
            inst,
            kIROp_VulkanRayPayloadInDecoration,
            getIntValue(getIntType(), location));
    }

    void addVulkanCallablePayloadDecoration(IRInst* inst, int location)
    {
        addDecoration(
            inst,
            kIROp_VulkanCallablePayloadDecoration,
            getIntValue(getIntType(), location));
    }

    void addVulkanCallablePayloadInDecoration(IRInst* inst, int location)
    {
        addDecoration(
            inst,
            kIROp_VulkanCallablePayloadInDecoration,
            getIntValue(getIntType(), location));
    }

    void addVulkanHitObjectAttributesDecoration(IRInst* inst, int location)
    {
        addDecoration(
            inst,
            kIROp_VulkanHitObjectAttributesDecoration,
            getIntValue(getIntType(), location));
    }

    void addGlobalVariableShadowingGlobalParameterDecoration(
        IRInst* inst,
        IRInst* globalVar,
        IRInst* key)
    {
        addDecoration(inst, kIROp_GlobalVariableShadowingGlobalParameterDecoration, globalVar, key);
    }

    void addMeshOutputDecoration(IROp d, IRInst* value, IRInst* maxCount)
    {
        SLANG_ASSERT(IRMeshOutputDecoration::isaImpl(d));
        // TODO: Ellie, correct int type here?
        addDecoration(value, d, maxCount);
    }

    void addKnownBuiltinDecoration(IRInst* value, KnownBuiltinDeclName enumValue)
    {
        addDecoration(
            value,
            kIROp_KnownBuiltinDecoration,
            getIntValue(getIntType(), IRIntegerValue(enumValue)));
    }

    void addKnownBuiltinDecoration(IRInst* value, UnownedStringSlice const& name)
    {
        auto enumValue = getKnownBuiltinDeclNameFromString(name);
        addDecoration(
            value,
            kIROp_KnownBuiltinDecoration,
            getIntValue(getIntType(), IRIntegerValue(enumValue)));
    }

    void addMemoryQualifierSetDecoration(IRInst* inst, IRIntegerValue flags)
    {
        addDecoration(inst, kIROp_MemoryQualifierSetDecoration, getIntValue(getIntType(), flags));
    }

    void addCheckpointIntermediateDecoration(IRInst* inst, IRGlobalValueWithCode* func)
    {
        addDecoration(inst, kIROp_CheckpointIntermediateDecoration, func);
    }

    void addEntryPointParamDecoration(IRInst* inst, IRFunc* entryPointFunc)
    {
        addDecoration(inst, kIROp_EntryPointParamDecoration, entryPointFunc);
    }

    void addRayPayloadDecoration(IRType* inst) { addDecoration(inst, kIROp_RayPayloadDecoration); }
};

// Helper to establish the source location that will be used
// by an IRBuilder.
struct IRBuilderSourceLocRAII
{
    IRBuilder* builder;
    SourceLoc sourceLoc;
    IRBuilderSourceLocRAII* next;

    IRBuilderSourceLocRAII(IRBuilder* builder, SourceLoc sourceLoc)
        : builder(builder), sourceLoc(sourceLoc), next(nullptr)
    {
        next = builder->getSourceLocInfo();
        builder->setSourceLocInfo(this);
    }

    ~IRBuilderSourceLocRAII()
    {
        SLANG_ASSERT(builder->getSourceLocInfo() == this);
        builder->setSourceLocInfo(next);
    }
};

// A helper to restore the builder's insert location on destruction
struct IRBuilderInsertLocScope
{
    IRBuilder* builder;
    IRInsertLoc insertLoc;
    IRBuilderInsertLocScope(IRBuilder* b)
        : builder(b), insertLoc(builder->getInsertLoc())
    {
    }
    ~IRBuilderInsertLocScope() { builder->setInsertLoc(insertLoc); }
};

//

void markConstExpr(IRBuilder* builder, IRInst* irValue);

//

IRTargetIntrinsicDecoration* findAnyTargetIntrinsicDecoration(IRInst* val);

template<typename T>
IRTargetSpecificDecoration* findBestTargetDecoration(IRInst* val, CapabilitySet const& targetCaps);

template<typename T>
IRTargetSpecificDecoration* findBestTargetDecoration(
    IRInst* val,
    CapabilityName targetCapabilityAtom);

bool findTargetIntrinsicDefinition(
    IRInst* callee,
    CapabilitySet const& targetCaps,
    UnownedStringSlice& outDefinition,
    IRInst*& outInst);

inline IRTargetIntrinsicDecoration* findBestTargetIntrinsicDecoration(
    IRInst* inInst,
    CapabilitySet const& targetCaps)
{
    return as<IRTargetIntrinsicDecoration>(
        findBestTargetDecoration<IRTargetSpecificDefinitionDecoration>(inInst, targetCaps));
}


void addHoistableInst(IRBuilder* builder, IRInst* inst);

} // namespace Slang
