#!/bin/sh

TARGET="$(basename $PWD)"

BUILD_DIR="build"
ASSET_DIR="assets"

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

replace() { echo "$1" | sed "s/\\$2/$3/"; }
addprefix() { echo "$1" | sed "s|^|$2|"; }

SRC=$(find . -type f -name "*.c" | sed 's|^\./||')
OBJS=$(replace "$SRC" .c .o)
OBJS=$(addprefix "$OBJS" "$BUILD_DIR/")

LSTS=$(addprefix "$(replace "$SRC" .c .lst)" "$BUILD_DIR/")

# TOOLS PATH
SDK=$PLAYDATE_SDK_PATH
GCC=$(which arm-none-eabi-gcc)
OBJCOPY=$(which arm-none-eabi-objcopy)
PDC=$SDK/bin/pdc
LINKER_SCRIPT=$SDK/C_API/buildsupport/link_map.ld

# COMPILER CONFIGURATION
DEFINES="-DTARGET_PLAYDATE=1 -DTARGET_EXTENSION=1" 
LIBS="-Ideps"

# -- CONFIGURATION ----
WARNINGS="-Wall" 
# WARNINGS="$WARNINGS -Wno-unused -Wstrict-prototypes -Wno-unknown-pragmas"
# WARNINGS="$WARNINGS -Wlto-type-mismatch -Wdouble-promotion"
DEBUG="-g3 -gdwarf-2 -fverbose-asm -Wa,-ahlms=$LSTS"
CPU="-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__FPU_USED=1"

# -- OPTIMIZATION ----
OPTS="-Ofast -flto -falign-functions=16 -fomit-frame-pointer"
OPTS="$OPTS -fsingle-precision-constant"

# -- COMPILER FLAGS ----
C_FLAGS="$LIBS $WARNINGS $DEBUG $CPU $OPTS $DEFINES"
C_FLAGS="$C_FLAGS -mword-relocations -fno-common"
C_FLAGS="$C_FLAGS -ffunction-sections -fdata-sections"
ASM_FLAGS="-D__HEAP_SIZE=8388208 -D__STACK_SIZE=61800 $CPU $DEBUG"
LINKER_FLAGS="-nostartfiles $CPU -T $LINKER_SCRIPT"
LINKER_FLAGS="$LINKER_FLAGS -Wl,-Map=$BUILD_DIR/pdex.map,--cref"
LINKER_FLAGS="$LINKER_FLAGS,--gc-sections,--no-warn-mismatch,--emit-relocs"
PDC_FLAGS="-sdkpath $SDK -q"

lst() {
  echo "$LSTS"
}

clean() {
  echo "$(basename $0): Cleaning up $BUILD_DIR/ and $TARGET.pdx"
  rm -rf $BUILD_DIR
  rm -rf $TARGET.pdx
}

assets() {
  echo "$(basename $0): Copying pdxinfo and static assets from $ASSET_DIR"
  mkdir -p $BUILD_DIR/Source/$ASSET_DIR
  cp -R $ASSET_DIR/* $BUILD_DIR/Source/$ASSET_DIR
  cp pdxinfo $BUILD_DIR/Source
}

OBJS_DONE=0
objs() {
  assets
  echo "$(basename $0): Building .o files"
  for file in $SRC; do
    mkdir -p $(dirname $BUILD_DIR/"${file%.c}.o")
    $GCC -c $C_FLAGS $file -o $BUILD_DIR/"${file%.c}.o"
  done
  OBJS_DONE=1
}

sim_bin() {
  if [ "$OBJS_DONE" = "0" ]; then 
    objs
  fi
  echo "$(basename $0): Building simulator executable"
	clang -g -dynamiclib -rdynamic -lm -DTARGET_SIMULATOR=1 -DTARGET_EXTENSION=1 -Ideps -o $BUILD_DIR/pdex.dylib $SRC
  cp $BUILD_DIR/pdex.dylib $BUILD_DIR/Source
}

dev_bin() {
  if [ "$OBJS_DONE" = "0" ]; then 
    objs
  fi
  echo "$(basename $0): Building device executable"
  $GCC $OBJS $LINKER_FLAGS -o $BUILD_DIR/pdex.elf
  cp $BUILD_DIR/pdex.elf $BUILD_DIR/Source
}

sim() {
  sim_bin
  $PDC $PDC_FLAGS $BUILD_DIR/Source $TARGET.pdx
  SIZE=$(ls -lh | grep $TARGET.pdx | awk '{ print $5 }')
  echo "$(basename $0): Packaging $TARGET.pdx ($SIZE)"
}

dev() {
  dev_bin
  $PDC $PDC_FLAGS $BUILD_DIR/Source $TARGET.pdx
  SIZE=$(ls -lh | grep $TARGET.pdx | awk '{ print $5 }')
  echo "$(basename $0): Packaging $TARGET.pdx ($SIZE)"
}

all() {
  sim_bin; dev_bin
  $PDC $PDC_FLAGS $BUILD_DIR/Source $TARGET.pdx
  SIZE=$(ls -lh | grep $TARGET.pdx | awk '{ print $5 }')
  echo "$(basename $0): Packaging $TARGET.pdx ($SIZE)"
}

run() {
  echo "$(basename $0): Running $TARGET.pdx"
  if [ -e $TARGET.pdx ]; then
    open -a "/Applications/Playdate Simulator" $TARGET.pdx
  else
    echo "$(basename $0): You must build $TARGET.pdx first"
  fi
}

for cmd in "$@"; do
  $cmd
done
