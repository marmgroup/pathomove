# check function
remove.packages("pathomove")

Rcpp::compileAttributes()
devtools::build(vignettes = FALSE)
{
  sink(file = "install_output.log")
  devtools::install(upgrade = "never", build_vignettes = FALSE)
  sink()
}
devtools::document()

detach(package:pathomove)
library(pathomove)
library(ggplot2)
library(data.table)

l <- pathomove::get_test_landscape(
  nItems = 1800,
  landsize = 60,
  nClusters = 60,
  clusterSpread = 1,
  regen_time = 50
)
ggplot(l) +
  geom_point(
    aes(x, y, col = tAvail)
    # size = 0.3
  ) +
  geom_segment(
    x = 0, y = 0,
    xend = 2, yend = 0
  ) +
  scale_colour_viridis_b(
    option = "H",
    direction = 1,
    breaks = c(0, 1, 2, 5, 10)
  ) +
  coord_equal()

# {t1 = Sys.time()
# invisible(
#   x = {
a <- pathomove::run_pathomove(
  scenario = 2,
  popsize = 1000,
  nItems = 1800,
  landsize = 60,
  nClusters = 60,
  clusterSpread = 1,
  tmax = 100,
  genmax = 1000,
  g_patho_init = 500,
  range_food = 1.0,
  range_agents = 1.0,
  range_move = 1.0,
  handling_time = 5,
  regen_time = 50,
  pTransmit = 0.05,
  initialInfections = 40,
  costInfect = 0.25,
  nThreads = 2,
  dispersal = 3.0,
  infect_percent = FALSE,
  mProb = 0.001,
  mSize = 0.001
)
#   }
# )
# t2 = Sys.time()
# t2 - t1}

# movement data
m1 <- a[["move_pre"]] |> rbindlist()
m2 <- a[["move_post"]] |> rbindlist()

m1_summary <- m1[, unlist(lapply(.SD, function(x) {
  list(
    first = first(x),
    last = last(x)
  )
}), recursive = F), by = c("id"), .SDcols = c("x", "y")]

m2_summary <- m2[, unlist(lapply(.SD, function(x) {
  list(
    first = first(x),
    last = last(x)
  )
}), recursive = F), by = c("id"), .SDcols = c("x", "y")]

ggplot(m1) +
  geom_point(
    aes(x, y, group = id, col = id),
    # size = 0.1
  ) +
  # geom_point(
  #   aes(x, y, col = id),
  #   # size = 0.1
  # )+
  scale_colour_viridis_c(
    option = "H"
  ) +
  coord_equal(
    # xlim = c(0, 50),
    # ylim = c(0, 50)
  )

data <- a
a <- data[[1]]
names(a)

plot(a[["gens"]], a[["n_infected"]], type = "o", pch = 16)

#### handle data ####
b <- copy(a)
b <- Map(function(l, g) {
  l$id <- seq(nrow(l))
  l$gen <- g
  l
}, b$pop_data, b$gens)
b <- rbindlist(b)
b
# b = b[(gen %% 100 == 0) | (gen == 9999),]

#### examine strategies ####
d <- copy(b)
d[, social_strat := fcase(
  (sH > 0 & sN > 0), "agent tracking",
  (sH > 0 & sN <= 0), "handler tracking",
  (sH <= 0 & sN > 0), "non-handler tracking",
  (sH <= 0 & sN <= 0), "agent avoiding"
)]

df <- d[, .N, by = c("gen", "social_strat")]

ggplot(df) +
  geom_point(
    aes(
      gen, N,
      col = social_strat
    )
  )

#### plot data ####
b <- melt(b, id.vars = c("gen", "id"))

ggplot(b[variable %in% c("intake", "moved")]) +
  stat_summary(
    aes(
      gen, value
    )
  ) +
  facet_wrap(~variable, scales = "free")

# energy = b[variable == "energy",]
wts <- b[!variable %in% c("energy", "assoc", "t_infec", "moved", "degree"), ]

#### explore network ####
library(igraph)
# g = data[["matrices"]][4:6]
# g = g[[3]]
setnames(b, "assoc", "weight")
g <- igraph::graph_from_data_frame(b[b$weight > 0, ], directed = FALSE)

plot(g, vertex.size = 3, vertex.label = NA)

library(tidygraph)
library(ggraph)

g <- tidygraph::as_tbl_graph(g)

ggraph(g, layout = "mds") +
  geom_node_point() +
  geom_edge_link()
