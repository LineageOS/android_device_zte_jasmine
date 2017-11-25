$(call inherit-product, device/zte/jasmine/full_jasmine.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

PRODUCT_NAME := lineage_jasmine
BOARD_VENDOR := zte
TARGET_VENDOR := zte
PRODUCT_DEVICE := jasmine

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_DEVICE="jasmine" \
    PRODUCT_NAME="K88" \
    BUILD_FINGERPRINT="ZTE/K88/jasmine:7.1.1/NMF26V/20170828.115541:user/release-keys" \
    PRIVATE_BUILD_DESC="K88-user 7.1.1 NMF26V 20170828.115541 release-keys"
