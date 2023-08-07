Get-ChildItem -Recurse -File -Path engine -Filter "*.fbs" | ForEach-Object {
    $GeneratedDirectory=Join-Path -Path $_.DirectoryName -ChildPath "generated"
    flatc --cpp --filename-suffix ".schema" -o $GeneratedDirectory $_.FullName
}
