import unreal

names = [n for n in dir(unreal) if "Factory" in n and ("Skeleton" in n or "Skeletal" in n or "Anim" in n or "Particle" in n)]
for n in sorted(names):
    unreal.log_warning(f"[FactorySymbol] {n}")
