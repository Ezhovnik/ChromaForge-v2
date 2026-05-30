local chromaforge_entities = {}

function chromaforge_entities.drop(ppos, itemid, count, ready)
    return entities.spawn("chromaforge:drop", ppos, {chromaforge__drop={
        id=itemid,
        count=count,
        ready=ready
    }})
end

return chromaforge_entities
