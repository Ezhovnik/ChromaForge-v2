local chromaforge_entities = {}

function chromaforge_entities.drop(ppos, itemid, count, pickup_delay)
    return entities.spawn("chromaforge:drop", ppos, {chromaforge__drop={
        id=itemid,
        count=count,
        pickup_delay=pickup_delay
    }})
end

return chromaforge_entities
