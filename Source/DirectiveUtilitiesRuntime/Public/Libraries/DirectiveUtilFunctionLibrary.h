// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilTypes.h"
#include "DirectiveUtilFunctionLibrary.generated.h"

/**
 * UDirectiveUtilFunctionLibrary
 *
 * The primary function library for the Directive Utilities plugin.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns a list of classes derived from the given base class (not limited to Actors).
	 * This exposes the built-in GetDerivedClasses function to blueprints.
	 * @param BaseClass The base class to get the derived classes for.
	 * @param bRecursive Whether to include derived classes of derived classes.
	 * @param DerivedClasses The list of derived classes.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Utility")
	static void GetChildClasses(const UClass* BaseClass, bool bRecursive, TArray<UClass*>& DerivedClasses);

	/**
	 * Copy the provided text to the clipboard.
	 * @param Text - The text to copy to the clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Clipboard" )
	static void CopyTextToClipboard(const FText& Text);

	/**
	 * Copy the provided string to the clipboard.
	 * @param String - The string to copy to the clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Clipboard" )
	static void CopyStringToClipboard(const FString& String);

	/**
	 * Get the content from the clipboard as FText.
	 * @returns The text from the clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Clipboard" )
	static FText GetTextFromClipboard();

	/**
	 * Get the content from the clipboard as an FString.
	 * @returns The content from the clipboard as a string.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Clipboard" )
	static FString GetStringFromClipboard();

	/**
	 * Clear the clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Clipboard")
	static void ClearClipboard();

	/**
	 * Get the project version as a string.
	 * @returns The project version as a string.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Utility")
	static FString GetProjectVersion();

	/**
	 * Returns true if the game is running in the Unreal Editor.
	 * @note This will return false in packaged/standalone builds.
	 * @returns True if running in the editor.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Utility")
	static bool IsRunningInEditor();

	/**
	 * Gets the type of world associated with the supplied context.
	 * @param WorldContextObject Object used to resolve the current world.
	 * @returns The resolved world type, or Unknown when the context has no world.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Utility", meta = (WorldContext = "WorldContextObject"))
	static EDirectiveUtilWorldType GetWorldType(const UObject* WorldContextObject);

	/**
	 * Gets the build configuration of the running application.
	 * @returns The active build configuration.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Utility", meta = (BlueprintThreadSafe))
	static EDirectiveUtilBuildConfiguration GetBuildConfigurationType();

	/**
	 * Gets the build target type of the running application.
	 * @returns The active build target type.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Utility", meta = (BlueprintThreadSafe))
	static EDirectiveUtilBuildTargetType GetBuildTargetType();

	/**
	 * Checks whether a switch (e.g. "MySwitch" matching "-MySwitch") was passed on the
	 * process command line. Matching is case-insensitive.
	 * @param Switch - The switch name, without the leading dash.
	 * @returns True if the switch is present.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Utility")
	static bool HasCommandLineSwitch(const FString& Switch);

	/**
	 * Reads a key=value option (e.g. "MyKey" matching "-MyKey=Value") from the process
	 * command line. Matching is case-insensitive; quoted values are returned without the quotes.
	 * @param Key - The key name, without the leading dash or trailing equals sign.
	 * @param OutValue - [out] The option's value, or empty if the key is missing.
	 * @returns True if the key was present.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Utility")
	static bool GetCommandLineOption(const FString& Key, FString& OutValue);

	/** Core of Has Command Line Switch that checks an explicit command line. */
	static bool HasCommandLineSwitch(const TCHAR* CommandLine, const FString& Switch);

	/** Core of Get Command Line Option that reads from an explicit command line. */
	static bool GetCommandLineOption(const TCHAR* CommandLine, const FString& Key, FString& OutValue);
};
