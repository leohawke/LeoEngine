template "LeoEngine Terrain File"
description "Terrain Infomation"
applies_to file
little-endian
read-only
multiple
begin
    float        "ChunkSize"
    uint32      "HorChunkNum"
    uint32       "VerChunkNum"
    char16[260]        "Height Map"
end